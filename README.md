📡 Classroom Noise Monitoring with Telegram Alerts

This project is a classroom noise monitoring system using an ESP32 and a MAX4466/MIC microphone sensor. The device measures sound levels in real time and sends an automatic alert to Telegram when the noise exceeds a defined threshold.

🔊 Features

Real-time noise level detection

Automatic Telegram notifications when the room becomes too loud

Optional OLED display (SSD1306) for live sound level status

Stable, non-blocking ESP32 operation with WiFi connectivity

🧩 Hardware

ESP32

MAX4466 / MAX9814 microphone module

OLED 0.96" SSD1306 (optional)

Jumper wires & breadboard

🧠 How It Works

The microphone reads the sound level.

ESP32 processes the signal and checks if it exceeds the threshold.

If the room is noisy, a Telegram message is sent automatically.

(Optional) OLED displays current noise level and system status.

📬 Telegram Integration

Uses a Bot Token and Chat ID from Telegram to send direct alerts—no server required.
