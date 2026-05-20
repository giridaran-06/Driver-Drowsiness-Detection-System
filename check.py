import cv2
import mediapipe as mp
import numpy as np
import serial
import time

# ══════════════════════════════════════════════
SERIAL_PORT     = "COM3"
BAUD_RATE       = 9600
SERIAL_INTERVAL = 3.0   # increased to 3 seconds — reduces spam
# ══════════════════════════════════════════════

def connect_serial():
    try:
        s = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)
        print(f"[OK] ESP32 connected on {SERIAL_PORT}")
        return s
    except Exception as e:
        print(f"[X] Serial failed: {e}")
        return None

def send_command(esp, cmd):
    if esp and esp.is_open:
        esp.write((cmd + '\n').encode())
        print(f"[->] Sent: {cmd}")
    else:
        print(f"[SIM] Would send: {cmd}")

def EAR(eye):
    A = np.linalg.norm(np.array(eye[1]) - np.array(eye[5]))
    B = np.linalg.norm(np.array(eye[2]) - np.array(eye[4]))
    C = np.linalg.norm(np.array(eye[0]) - np.array(eye[3]))
    return (A + B) / (2.0 * C)

def main():
    esp = connect_serial()

    mp_face_mesh = mp.solutions.face_mesh
    face_mesh    = mp_face_mesh.FaceMesh(refine_landmarks=True)

    LEFT_EYE  = [33, 160, 158, 133, 153, 144]
    RIGHT_EYE = [362, 385, 387, 263, 373, 380]

    cap           = cv2.VideoCapture(0)
    counter       = 0
    last_status   = ""
    last_serial_t = 0

    print("[OK] Monitoring started. ESC to quit.\n")

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        h, w    = frame.shape[:2]
        rgb     = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        results = face_mesh.process(rgb)

        status = "NO_FACE"
        color  = (255, 100, 0)

        if results.multi_face_landmarks:
            for face_landmarks in results.multi_face_landmarks:
                left_eye, right_eye = [], []

                for idx in LEFT_EYE:
                    x = int(face_landmarks.landmark[idx].x * w)
                    y = int(face_landmarks.landmark[idx].y * h)
                    left_eye.append((x, y))

                for idx in RIGHT_EYE:
                    x = int(face_landmarks.landmark[idx].x * w)
                    y = int(face_landmarks.landmark[idx].y * h)
                    right_eye.append((x, y))

                ear = (EAR(left_eye) + EAR(right_eye)) / 2.0

                for (x, y) in left_eye + right_eye:
                    cv2.circle(frame, (x, y), 2, (0, 255, 0), -1)

                # ── Counter ────────────────────
                if ear < 0.23:
                    counter += 1
                else:
                    counter = 0

                # ── Status Decision ────────────
                if counter > 40:
                    status = "ALERT"
                    color  = (0, 0, 255)
                elif counter > 20:
                    status = "WARNING"
                    color  = (0, 165, 255)
                else:
                    status = "SAFE"
                    color  = (0, 255, 0)

                cv2.putText(frame, f"EAR: {ear:.2f}", (30, 50),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)

        else:
            # ── No face — reset counter → SAFE
            # never send WARNING or ALERT when no face
            counter = 0
            status  = "NO_FACE"
            color   = (255, 100, 0)

        now = time.time()

        # ── Send to ESP32 only on status change ─
        # removes periodic resend — only sends when
        # something actually changes
        if status != last_status:
            send_command(esp, status)
            last_serial_t = now
            last_status   = status

        # ── Display ────────────────────────────
        cv2.putText(frame, status, (30, 100),
            cv2.FONT_HERSHEY_SIMPLEX, 1.2, color, 3)

        cv2.rectangle(frame, (0, h - 40), (w, h), color, -1)
        cv2.putText(frame,
            f"Status: {status}  |  Counter: {counter}",
            (10, h - 12),
            cv2.FONT_HERSHEY_SIMPLEX, 0.55, (0, 0, 0), 2)

        cv2.imshow("Drowsiness Monitor", frame)
        if cv2.waitKey(1) & 0xFF == 27:
            break

    cap.release()
    cv2.destroyAllWindows()
    if esp:
        send_command(esp, "SAFE")
        esp.close()

if __name__ == "__main__":
    main()