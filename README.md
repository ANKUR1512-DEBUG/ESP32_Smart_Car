# 🚗 ESP32 Smart Car

A Wi-Fi controlled smart car built using an ESP32, L298N motor driver, ultrasonic sensor, buzzer, headlight and ESP32-CAM.

## Features

- Forward / Backward / Left / Right / Stop
- Hold-to-drive controls
- Mobile-friendly web controller
- ESP32-CAM live video
- Horn
- Headlight ON/OFF
- Ultrasonic reverse obstacle protection
- Wi-Fi control

## Project structure

```text
ESP32-Smart-Car/
├── SmartCar/
│   └── SmartCar.ino
├── ESP32-CAM/
│   └── ESP32CAM.ino
├── Web-Controller/
│   ├── index.html
│   └── camera.jpg
├── images/
│   └── controller-ui.png
└── README.md
```

## ESP32 WROOM pin mapping

| Component | ESP32 |
|---|---|
| L298N IN1 | GPIO26 |
| L298N IN2 | GPIO27 |
| L298N IN3 | GPIO14 |
| L298N IN4 | GPIO13 |
| Buzzer | GPIO25 |
| Ultrasonic TRIG | GPIO33 |
| Ultrasonic ECHO | GPIO34 |
| Headlight | GPIO32 |

## L298N motors

- Left motor(s): OUT1 / OUT2
- Right motor(s): OUT3 / OUT4

If a motor spins in the opposite direction, swap that motor's two wires.

## Wi-Fi setup

Both ESP32 and ESP32-CAM should connect to the same Wi-Fi/hotspot.

In both sketches replace:

```cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
```

with your own credentials.

**Do not commit your real Wi-Fi password to a public GitHub repository.**

The ESP32 controller prints its IP in Serial Monitor.

The ESP32-CAM prints its IP and stream URL in Serial Monitor:

```text
http://CAMERA_IP:81/stream
```

## Camera

The Web Controller contains `camera.jpg` so the UI can be demonstrated offline.

For the real ESP32-CAM stream, change the image source to:

```text
http://CAMERA_IP:81/stream
```

## Power

Recommended basic arrangement for a 2-cell Li-ion pack:

```text
2 × Li-ion in series
        |
      switch
        |
      L298N
```

Use an appropriate regulated 5 V supply for the ESP32.

**Do not connect three Li-ion cells in series directly to the ESP32.**

### Ultrasonic warning

If using an HC-SR04, its ECHO signal may be 5 V. ESP32 GPIOs are 3.3 V logic, so use a voltage divider or suitable level shifter before GPIO34.

## Running the web UI

Open:

```text
Web-Controller/index.html
```

The UI runs in demo mode by default.

For real controller requests, edit:

```js
const REAL_MODE = true;
const CONTROLLER_IP = "YOUR_ESP32_CONTROLLER_IP";
```

## License

Add the license you prefer before publishing.
