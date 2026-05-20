#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ───── CONFIG ─────
const char* WIFI_SSID = "YOUR_WI-FI_USERENAME";
const char* WIFI_PASS = "YOUR_WI-FI_PASSWORD";
const char* BOT_TOKEN = "YOUR_BOT-TOKEN";
const char* CHAT_ID   = "YOUR_CHAT_ID";

// ───── PINS ─────
#define LED_PIN     2
#define BUZZER_PIN  4

// ───── LCD ─────
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ───── STATE ─────
int           warnCount      = 0;
unsigned long firstWarnTime  = 0;
unsigned long lastAlertTime  = 0;
unsigned long lastWiFiCheck  = 0;
bool          buzzerActive   = false;  // tracks if buzzer is running

#define WARN_COUNT_LIMIT   3
#define WARN_WINDOW        300000   // 5 minutes
#define ALERT_COOLDOWN     30000    // 30 seconds
#define WIFI_CHECK_MS      10000    // 10 seconds

// ───── TELEGRAM ─────
void sendTelegram(String msg) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[X] No WiFi - Telegram skipped");
    return;
  }
  HTTPClient http;
  String url = "https://api.telegram.org/bot"
               + String(BOT_TOKEN) + "/sendMessage";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"chat_id\":\"" + String(CHAT_ID) + "\","
                "\"text\":\"" + msg + "\","
                "\"parse_mode\":\"Markdown\"}";

  int code = http.POST(json);
  Serial.println(code == 200
    ? "[OK] Telegram sent"
    : "[X] Telegram fail: " + String(code));
  http.end();
}

// ───── LCD SCREENS ─────
void showReady() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready    ");
  lcd.setCursor(0, 1);
  lcd.print("Waiting...      ");
}

void showSafe() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Status: SAFE    ");
  lcd.setCursor(0, 1);
  lcd.print("Driver Alert    ");
}

void showWarning(int count) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WARNING!        ");
  lcd.setCursor(0, 1);
  lcd.print("Eyes Closing ");
  lcd.print(count);
  lcd.print("/");
  lcd.print(WARN_COUNT_LIMIT);
}

void showAlert() {
  for (int i = 0; i < 3; i++) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!!! ALERT !!!");
    lcd.setCursor(0, 1);
    lcd.print("Driver Drowsy");
    delay(400);
    lcd.clear();
    delay(200);
  }
  lcd.setCursor(0, 0);
  lcd.print("!!! ALERT !!!");
  lcd.setCursor(0, 1);
  lcd.print("Msg Sent!       ");
}

void showNoFace() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("No Face Found   ");
  lcd.setCursor(0, 1);
  lcd.print("Adjust Camera   ");
}

// ───── BUZZER ─────

void buzzerOff() {
  // stop buzzer immediately — called when driver wakes up
  digitalWrite(BUZZER_PIN, LOW);
  buzzerActive = false;
  Serial.println("[~] Buzzer stopped");
}

// slow 3 beeps — warning sound
// different from alert — slower and softer
void buzzerWarning() {
  buzzerActive = true;
  Serial.println("[~] Warning buzzer");
  for (int i = 0; i < 3; i++) {
    if (!buzzerActive) break;   // stop if driver woke up
    digitalWrite(BUZZER_PIN, HIGH); delay(400);
    digitalWrite(BUZZER_PIN, LOW);  delay(300);
  }
}

// fast continuous beeps — alert sound
// clearly different from warning — faster and longer
void buzzerAlert() {
  buzzerActive = true;
  Serial.println("[~] Alert buzzer");
  for (int i = 0; i < 20; i++) {
    if (!buzzerActive) break;   // stop if driver woke up
    digitalWrite(BUZZER_PIN, HIGH); delay(100);
    digitalWrite(BUZZER_PIN, LOW);  delay(60);
  }
}

// ───── WIFI ─────
void connectWiFi() {
  lcd.clear();
  lcd.print("Connecting WiFi ");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[OK] WiFi connected");
    lcd.clear();
    lcd.print("WiFi Connected! ");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP().toString());
    delay(2000);
  } else {
    Serial.println("[X] WiFi failed");
    lcd.clear();
    lcd.print("WiFi Failed!    ");
    lcd.setCursor(0, 1);
    lcd.print("Offline Mode    ");
    delay(2000);
  }
}

// ───── HANDLE STATUS ─────
void handleStatus(String status) {
  unsigned long now = millis();

  // ── SAFE — driver woke up ──────────────────
  if (status == "SAFE") {
    buzzerOff();              // stop any buzzer immediately
    digitalWrite(LED_PIN, LOW);
    showSafe();
    Serial.println("[OK] SAFE");
  }

  // ── WARNING ───────────────────────────────
  else if (status == "WARNING") {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);

    if (warnCount == 0) {
      firstWarnTime = now;
    }

    if ((now - firstWarnTime) < WARN_WINDOW) {
      warnCount++;
      Serial.println("[!] Warn count: "
        + String(warnCount)
        + "/" + String(WARN_COUNT_LIMIT));

      showWarning(warnCount);

      // ── buzzer starts after 2nd warning ────
      if (warnCount >= 2) {
        buzzerWarning();
      }

      // ── telegram after 3rd warning ─────────
      if (warnCount >= WARN_COUNT_LIMIT) {
        sendTelegram(
          "⚠️ *WARNING ALERT*\n"
          "━━━━━━━━━━━━━━━\n"
          "😪 Driver drowsy 3 times in 5 mins!\n"
          "👀 Eyes closing repeatedly\n"
          "🙏 Please check on the driver!"
        );
        warnCount     = 0;
        firstWarnTime = 0;
      }

    } else {
      // window expired — reset count
      Serial.println("[!] Warn window expired - reset");
      warnCount     = 1;
      firstWarnTime = now;
      showWarning(warnCount);
    }
  }

  // ── ALERT ─────────────────────────────────
  else if (status == "ALERT") {
    // loud fast buzzer — always on alert
    buzzerAlert();
    digitalWrite(LED_PIN, HIGH);
    showAlert();
    Serial.println("[!!] ALERT");

    // telegram immediately on first alert
    // then 30 second cooldown
    if (now - lastAlertTime > ALERT_COOLDOWN) {
      sendTelegram(
        "🚨 *EMERGENCY ALERT* 🚨\n"
        "━━━━━━━━━━━━━━━\n"
        "😴 Driver is DROWSY!\n"
        "🔴 Eyes fully closed!\n"
        "🛑 PULL OVER IMMEDIATELY!\n"
        "⛔ DO NOT IGNORE THIS!"
      );
      lastAlertTime = now;
      warnCount     = 0;
      firstWarnTime = 0;
    }
  }

  // ── NO FACE ───────────────────────────────
  else if (status == "NO_FACE") {
    buzzerOff();
    digitalWrite(LED_PIN, LOW);
    showNoFace();
    Serial.println("[?] NO_FACE");
  }

  else {
    Serial.println("[?] Unknown: [" + status + "]");
  }
}

// ───── SETUP ─────
void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN,    OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  buzzerOff();
  digitalWrite(LED_PIN, LOW);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.print("System Starting ");
  delay(1000);

  connectWiFi();

  showReady();
  Serial.println("[OK] ESP32 Ready.");
}

// ───── LOOP ─────
void loop() {

  // ── WiFi watchdog non blocking ─────────────
  unsigned long now = millis();
  if (now - lastWiFiCheck > WIFI_CHECK_MS) {
    lastWiFiCheck = now;
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[!] WiFi lost - reconnecting");
      WiFi.reconnect();
    }
  }

  // ── Read from Python ──────────────────────
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() > 0) {
      Serial.println("[<-] " + cmd);
      handleStatus(cmd);
    }
  }
}