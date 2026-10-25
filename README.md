# 🌡️ Vigilant Potato — ESP32 IoT Monitor

An embedded IoT project for monitoring temperature and relative humidity with an **ESP32** and **DHT22** sensor. The firmware displays readings on an **SSD1306 OLED** and sends telemetry to **Blynk Cloud** and an **MQTT broker**.

Built with Arduino C++ as a hands-on project in embedded systems, sensor interfacing, Wi-Fi connectivity, and IoT messaging.

## ✨ Features

- Reads temperature and relative humidity from a DHT22 sensor.
- Displays measurements on a 128 × 64 SSD1306 OLED.
- Sends temperature and humidity to Blynk virtual pins `V0` and `V1`.
- Publishes sensor readings as JSON over MQTT.
- Includes connection retry logic for Wi-Fi, MQTT, and Blynk.
- Provides Serial Monitor output for debugging and connection status.

## 🧱 Technology stack

| Technology | Purpose |
|---|---|
| ESP32 | Microcontroller and Wi-Fi |
| DHT22 | Temperature and humidity sensing |
| SSD1306 OLED | Local display |
| Arduino C++ | Firmware |
| Blynk Cloud | Remote dashboard |
| MQTT / PubSubClient | Telemetry messaging |
| I²C | OLED communication |
| Wokwi | Simulation and development |

## 🏗️ System overview

```text
DHT22 sensor ──GPIO 4──> ESP32
                            ├── I²C (SDA 21, SCL 22) ──> SSD1306 OLED
                            └── Wi-Fi ──> Blynk Cloud
                                      └─> MQTT broker
```

The ESP32 reads the sensor, refreshes the local display, and attempts to send readings to the configured IoT services. Remote publishing depends on network access and service availability.

## 🔌 Wiring

### DHT22

| DHT22 pin | ESP32 connection |
|---|---|
| VCC | 3V3 |
| DATA | GPIO 4 |
| GND | GND |

### SSD1306 OLED

| OLED signal | ESP32 connection |
|---|---|
| VCC | 3V3, subject to module specifications |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

**Display configuration:** SSD1306, 128 × 64 pixels, I²C address `0x3C`.

> **Compatibility note:** The current sketch uses the SSD1306 driver. If you use a Grove SH1107 display instead, the display driver and initialization may need to be changed to match that hardware.

## 📁 Repository structure

```text
vigilant-potato/
├── .gitignore
├── README.md
└── VigilantPotato/
    ├── VigilantPotato.ino
    ├── LIBRARIES.md
    └── secrets.h.example
```

- `VigilantPotato/VigilantPotato.ino` — main Arduino firmware.
- `VigilantPotato/LIBRARIES.md` — library setup notes.
- `VigilantPotato/secrets.h.example` — example for private configuration.
- `.gitignore` — excludes designated local secret files from version control.

## 🧰 Requirements

- Arduino IDE 2.x
- ESP32 board support package by Espressif Systems
- Blynk library
- DHT sensor library by Adafruit
- Adafruit Unified Sensor
- Adafruit GFX Library
- Adafruit SSD1306
- PubSubClient

See [LIBRARIES.md](VigilantPotato/LIBRARIES.md) for project-specific installation notes.

## 🚀 Setup

### 1. Get the code

```bash
git clone https://github.com/thooyavan-a05z/vigilant-potato.git
cd vigilant-potato
```

Open `VigilantPotato/VigilantPotato.ino` in Arduino IDE and install the required board support and libraries.

### 2. Configure Blynk

1. Create or open a Blynk template and device.
2. Set the template ID and name to match your Blynk template.
3. Configure numeric datastreams: `V0` for temperature in °C and `V1` for relative humidity in %.
4. Add dashboard widgets for those datastreams.
5. Configure your device authentication token locally.

The sketch currently contains the placeholder `PASTE_YOUR_NEW_BLYNK_TOKEN_HERE`. Replace it only in your local working copy. **Never commit a real device token to this public repository.** A token was exposed during development, so regenerate it in Blynk before using the project.

### 3. Configure Wi-Fi

The current sketch is configured for Wokwi's guest network:

- SSID: `Wokwi-GUEST`
- Password: empty

For physical hardware, use your own network credentials locally. Do not push private Wi-Fi credentials to GitHub.

### 4. MQTT configuration

| Setting | Value |
|---|---|
| Broker | `test.mosquitto.org` |
| Port | `1883` |
| Topic | `vigilant-potato/esp32/telemetry` |

The configured broker connection uses unencrypted MQTT on port 1883. This public broker is suitable only for demonstrations; do not publish sensitive data.

Example payload (illustrative values):

```json
{
  "temperature": 27.5,
  "humidity": 61.2
}
```

### 5. Compile and test

1. Select the appropriate ESP32 board in Arduino IDE.
2. Click **Verify** to compile the sketch.
3. For hardware testing, connect the circuit and upload the firmware.
4. Open Serial Monitor at **115200 baud**.
5. Check sensor readings, OLED output, Wi-Fi connection, MQTT publishing, and Blynk status separately.

A successful compile does not guarantee that the sensor, display, or cloud connections are all working. The project has been used in Wokwi and verified to compile in Arduino IDE; cloud-service availability may vary.

## 📊 Blynk datastreams

| Virtual pin | Measurement | Unit |
|---|---|---|
| `V0` | Temperature | °C |
| `V1` | Relative humidity | % |

Make sure the virtual pins and datastream types in Blynk match the firmware.

## 🛠️ Troubleshooting

| Symptom | Checks |
|---|---|
| No DHT22 readings | Check power, ground, GPIO 4, sensor type, and Serial Monitor output |
| OLED is blank | Check wiring, I²C address `0x3C`, power, and SSD1306 compatibility |
| Wi-Fi won't connect | Check the SSID/password and network or simulator configuration |
| MQTT publish fails | Check internet access, broker availability, port, topic, and logs |
| Blynk is offline | Check device token, template ID/name, datastreams, and network access |

## 🔐 Security

- Regenerate any previously exposed Blynk token before use.
- Keep real tokens and Wi-Fi passwords out of tracked files and commit history.
- Verify that local secret files are ignored by Git before committing.
- Do not send sensitive data through the public MQTT broker.
- For a production deployment, use authenticated MQTT with TLS and appropriate credential management.

## 🗺️ Possible improvements

- [ ] Add a Wokwi simulation link and real screenshots of the circuit and dashboard.
- [ ] Move credentials from the main sketch into a local, Git-ignored configuration file.
- [ ] Add clear sensor-error and network-status indicators.
- [ ] Improve non-blocking reconnect behavior.
- [ ] Add historical charts or alerts to the dashboard.
- [ ] Evaluate a secure MQTT broker for deployment.

## 🎓 What this project demonstrates

- ESP32 firmware development with Arduino C++
- Digital sensor interfacing and I²C communication
- Wi-Fi troubleshooting
- MQTT telemetry and cloud dashboard integration
- Embedded debugging and configuration hygiene

## 👨‍💻 Author

**Thooyavan** · [GitHub profile](https://github.com/thooyavan-a05z)

**Repository:** [thooyavan-a05z/vigilant-potato](https://github.com/thooyavan-a05z/vigilant-potato)

---

*An embedded systems and IoT learning project.*
