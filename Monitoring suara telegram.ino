#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "time.h"
#include <math.h>

// === PIN ===
#define MIC_PIN 34          // pin A0 dari GY-MAX4466 ke GPIO34
#define BUZZER_PIN 25

// === OLED SH1106 ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// === WIFI ===
const char* ssid = "ssid";
const char* password = "password";

// === TELEGRAM ===
String BOT_TOKEN = "token fatherbot";
String CHAT_ID = "idbot";

// === NTP ===
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;

// === Variabel kontrol ===
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 60000; // kirim tiap 60 detik
const float batasKebisingan = 70.0;       // batas dB
float lastSentDb = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // === OLED ===
  Wire.begin();
  if (!display.begin(0x3C, true)) {
    Serial.println("Gagal inisialisasi OLED SH1106");
    while (1);
  }
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Menghubungkan WiFi...");
  display.display();

  // === WIFI ===
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung");

  // === NTP ===
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Sinkronisasi waktu NTP...");
}

// Fungsi untuk hitung rata-rata nilai mikrofon
float bacaRataSuara(int sampel) {
  long total = 0;
  for (int i = 0; i < sampel; i++) {
    total += analogRead(MIC_PIN);
    delayMicroseconds(200);
  }
  return (float)total / sampel;
}

void loop() {
  // === Baca nilai mikrofon ===
  float micValue = bacaRataSuara(100);
  float voltage = micValue * (3.3 / 4095.0);

  // Konversi ke dB (perkiraan)
  float dB = 20 * log10(voltage / 0.00631);
  if (dB < 30) dB = 30;
  if (dB > 100) dB = 100;

  // === Ambil waktu NTP ===
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Gagal ambil waktu");
    return;
  }
  char waktu[20];
  strftime(waktu, sizeof(waktu), "%H:%M:%S", &timeinfo);

  // === Status kebisingan ===
  String status;
  if (dB < 50) status = "Tenang";
  else if (dB < 70) status = "Sedang";
  else status = "Berisik!";

  // === Serial monitor ===
  Serial.printf("[%s] Mic: %.0f | %.1f dB | %s\n", waktu, micValue, dB, status.c_str());

  // === OLED Display ===
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.printf("Waktu: %s\n", waktu);
  display.setCursor(0, 16);
  display.printf("Kebisingan: %.1f dB", dB);
  display.setCursor(0, 32);
  display.printf("Status: %s", status.c_str());

  // === Bar visual ===
  int barWidth = map((int)dB, 30, 100, 0, 120);
  display.drawRect(4, 50, 120, 10, SH110X_WHITE);
  display.fillRect(4, 50, barWidth, 10, SH110X_WHITE);
  display.display();

  // === Buzzer ===
  if (dB > batasKebisingan) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  // === Kirim Telegram tiap 60 detik ===
  if (millis() - lastSendTime > sendInterval) {
    kirimTelegram(waktu, dB, status);
    lastSendTime = millis();
    lastSentDb = dB;
  }

  delay(1000);
}

// === Kirim pesan ke Telegram ===
void kirimTelegram(String waktu, float dB, String status) {
  if (WiFi.status() != WL_CONNECTED) return;

  String emoji = "🔈";
  if (status == "Sedang") emoji = "🔉";
  if (status == "Berisik!") emoji = "🔊🚨";

  String message = emoji + String(" *Monitor Suara Kelas*\n");
  message += "Waktu: " + waktu + "\n";
  message += "Kebisingan: " + String(dB, 1) + " dB\n";
  message += "Status: *" + status + "*";

  HTTPClient http;
  String url = "https://api.telegram.org/bot" + BOT_TOKEN +
               "/sendMessage?chat_id=" + CHAT_ID +
               "&text=" + urlencode(message) +
               "&parse_mode=Markdown";

  http.begin(url);
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    Serial.println("Pesan Telegram terkirim!");
  } else {
    Serial.printf("Gagal kirim Telegram: %d\n", httpResponseCode);
  }
  http.end();
}

// === URL Encode ===
String urlencode(String str) {
  String encodedString = "";
  char c;
  char code1, code2;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encodedString += c;
    } else {
      encodedString += '%';
      code1 = (c >> 4) & 0xF;
      code2 = c & 0xF;
      code1 += (code1 > 9) ? 'A' - 10 : '0';
      code2 += (code2 > 9) ? 'A' - 10 : '0';
      encodedString += code1;
      encodedString += code2;
    }
  }
  return encodedString;
}
