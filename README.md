# Vigilant Potato 🌡️

An embedded IoT temperature and humidity monitor built with an ESP32, DHT22 sensor, SSD1306 128×64 OLED, Wi-Fi, Blynk Cloud, and MQTT.

## Features

- Reads temperature and relative humidity from a DHT22 sensor.
- Displays readings on an SSD1306 128×64 OLED.
- Sends temperature and humidity to Blynk Cloud using virtual pins V0 and V1.
- Publishes readings as JSON over MQTT.
- Retries Wi-Fi, MQTT, and Blynk connections.
- Arduino IDE / C++ sketch designed for Wokwi simulation and ESP32 development boards.

## Repository structure

```text
VigilantPotato/
├── VigilantPotato.ino
├── LIBRARIES.md
└── secrets.h.example
```

- `VigilantPotato/VigilantPotato.ino` — main Arduino sketch.
- `VigilantPotato/LIBRARIES.md` — required libraries and setup.
- `VigilantPotato/secrets.h.example` — optional reference for storing a token privately; the current sketch uses a token placeholder directly in the main file.

## Hardware and pin connections

| Component | Signal | ESP32 pin |
|---|---|---|
| DHT22 | DATA | GPIO 4 |
| DHT22 | VCC | 3V3 |
| DHT22 | GND | GND |
| SSD1306 OLED | SDA | GPIO 21 |
| SSD1306 OLED | SCL | GPIO 22 |
| SSD1306 OLED | VCC | 3V3 (check your module requirements) |
| SSD1306 OLED | GND | GND |

OLED I²C address: `0x3C`.

## Requirements

- Arduino IDE 2
- ESP32 board support by Espressif Systems
- Blynk library
- DHT sensor library by Adafruit
- Adafruit Unified Sensor
- Adafruit GFX Library
- Adafruit SSD1306
- PubSubClient

See [LIBRARIES.md](VigilantPotato/LIBRARIES.md) for installation instructions.

## Configure Blynk

1. Open `VigilantPotato/VigilantPotato.ino`.
2. Replace `PASTE_YOUR_NEW_BLYNK_TOKEN_HERE` in the `BLYNK_AUTH_TOKEN` definition with your own Blynk device token.
3. Keep your token private. Do not commit a real token to a public repository; replace it with the placeholder before pushing changes.
4. In Blynk, create numeric datastreams **V0** for temperature (°C) and **V1** for humidity (%), and add suitable dashboard widgets.
5. Confirm that the template ID/name in the sketch matches your Blynk template.

A token was exposed during development; regenerate it in Blynk before use.

## Wi-Fi settings

The sketch is configured for Wokwi's guest network:

- SSID: `Wokwi-GUEST`
- Password: empty

For physical hardware, replace these with your local Wi-Fi credentials locally. Do not publish private Wi-Fi credentials.

## MQTT settings

- Broker: `test.mosquitto.org`
- Port: `1883` (unencrypted)
- Topic: `vigilant-potato/esp32/telemetry`

Example JSON payload:

```json
{"temperature":27.5,"humidity":61.2}
```

This is a public demonstration broker, not a private or secure production service. Do not send sensitive information through it.

## Build and test

1. Install ESP32 board support and the libraries listed in `VigilantPotato/LIBRARIES.md`.
2. Replace the Blynk token placeholder with your own token for local testing.
3. Open `VigilantPotato/VigilantPotato.ino` in Arduino IDE and select the correct ESP32 board.
4. Click **Verify** to compile, then upload to a physical board if available.
5. Open Serial Monitor at **115200 baud** and check the OLED, DHT22, Wi-Fi, MQTT, and Blynk messages.

The project owner reports that the sketch works in Wokwi and verifies successfully in Arduino IDE. A successful compile alone does not confirm that every cloud connection is online.

## Security

Never commit a real Blynk token, private Wi-Fi password, or other credentials. The root `.gitignore` excludes `VigilantPotato/secrets.h`; keep any local secret file untracked. The MQTT demo broker is public and unencrypted.
