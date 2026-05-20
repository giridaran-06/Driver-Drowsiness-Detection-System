# Driver Drowsiness Detection System 🚗💤

An AI-powered real-time driver monitoring and alert system developed using Python, OpenCV, MediaPipe, and ESP32 to detect driver drowsiness and improve road safety.

This system continuously monitors the driver's eyes using Computer Vision and triggers alerts when drowsiness is detected. The project combines AI software + embedded hardware + IoT communication into one complete safety solution.

---

# 📌 Project Objective

Driver fatigue is one of the leading causes of road accidents worldwide.

The goal of this project is to create a low-cost, real-time safety system capable of detecting driver drowsiness before accidents occur.

The system:
- Detects eye closure in real time
- Calculates Eye Aspect Ratio (EAR)
- Identifies warning and emergency states
- Activates buzzer and LED alerts
- Displays live status on LCD
- Sends Telegram emergency notifications remotely

---

# ⚡ Features

✅ Real-time eye tracking using webcam  
✅ Drowsiness detection using EAR algorithm  
✅ MediaPipe Face Mesh integration  
✅ ESP32 hardware communication  
✅ Telegram emergency alert system  
✅ LCD live status display  
✅ Audio buzzer alerts  
✅ LED visual indication  
✅ Lightweight CPU-based processing  
✅ No GPU required  
✅ Low-cost implementation

---

# 🧠 Technologies Used

| Technology | Purpose |
|---|---|
| Python | Main processing |
| OpenCV | Video capture and image processing |
| MediaPipe Face Mesh | Facial landmark detection |
| NumPy | Mathematical calculations |
| PySerial | Serial communication |
| ESP32 | Hardware controller + WiFi |
| Arduino IDE | ESP32 programming |
| Telegram Bot API | Remote notifications |

---

# 🔧 Hardware Components

| Component | Purpose |
|---|---|
| ESP32 | Controls hardware and sends Telegram alerts |
| Webcam | Captures driver's face |
| 16x2 I2C LCD | Displays live driver status |
| Active Buzzer | Audio warning system |
| LED | Visual status indication |
| USB Serial Communication | Python ↔ ESP32 communication |

---

# 👁️ Working Principle

## 1. Face Detection
The webcam captures live video frames continuously.

MediaPipe Face Mesh detects 468 facial landmarks in real time.

Eye landmarks used:

LEFT_EYE  = [33, 160, 158, 133, 153, 144]
RIGHT_EYE = [362, 385, 387, 263, 373, 380]

---

## 2. Eye Aspect Ratio (EAR)

The Eye Aspect Ratio is calculated to determine whether the driver's eyes are open or closed.

Formula used:

EAR = (A + B) / (2.0 * C)

### EAR Logic
- Eyes Open → Higher EAR value
- Eyes Closed → Lower EAR value

Threshold used:
EAR < 0.23

---

# 🚨 Drowsiness Detection Logic

The system uses a frame counter to avoid false alarms caused by normal blinking.

| Counter Value | Status |
|---|---|
| Normal | SAFE |
| Counter > 20 | WARNING |
| Counter > 40 | ALERT |

---

# 📡 Python ↔ ESP32 Communication

Python sends real-time status updates to ESP32 through Serial Communication.

Commands sent:
- SAFE
- WARNING
- ALERT
- NO_FACE

Example:
send_command(esp, status)

---

# 📲 Telegram Alert System

ESP32 connects to WiFi and sends emergency notifications through Telegram Bot API.

### ⚠️ WARNING ALERT
Triggered after repeated drowsiness events.

### 🚨 EMERGENCY ALERT
Triggered when the driver's eyes remain fully closed.

Example:

🚨 EMERGENCY ALERT 🚨
😴 Driver is DROWSY!
🔴 Eyes fully closed!
🛑 PULL OVER IMMEDIATELY!

---

# 🔊 Hardware Alert System

## LCD Display
Displays:
- SAFE
- WARNING
- ALERT
- NO FACE

## Buzzer
- Slow beep → WARNING
- Fast continuous beep → ALERT

## LED
Visual indication for warning states.

---

# 💡 Why ESP32 Was Used

ESP32 was chosen because:
- Built-in WiFi support
- Fast processing
- IoT capability
- Telegram integration support
- Better suited for embedded IoT projects than Arduino Uno

---

# 📷 System Workflow

Webcam → OpenCV → MediaPipe Face Mesh
        ↓
EAR Calculation
        ↓
Drowsiness Detection
        ↓
Serial Communication
        ↓
ESP32
        ↓
LCD + Buzzer + Telegram Alerts

---

# 📈 Advantages

✅ Low-cost implementation  
✅ Real-time monitoring  
✅ Lightweight processing  
✅ Easy deployment  
✅ Can run on normal laptops  
✅ Useful for transport safety systems

---

# 🚀 Future Improvements

- GPS location sharing
- GSM emergency calling
- Mobile application integration
- Cloud dashboard monitoring
- AI-based head pose estimation
- Night vision support
- Driver fatigue analytics

---

# 📌 Applications

- Trucks
- Buses
- Taxi services
- Fleet management
- Industrial vehicle safety
- Long-distance transportation

---

# 👨‍💻 Developed By

Giridaran K  
B.Tech Mechatronics Engineer


---

# 🛡️ Goal of the Project

To reduce accidents caused by driver fatigue using affordable AI-powered driver safety technology.
