# Vigilant Potato 🌡️

An ESP32-based temperature and humidity monitor built with **Arduino IDE (C++)**, a **DHT22**, an **I²C OLED**, **Wi-Fi**, **Blynk Cloud**, and **MQTT**.

## Features

- Reads temperature and relative humidity from a DHT22 sensor.
- Shows readings on an OLED display.
- Sends temperature and humidity to Blynk Cloud.
- Publishes readings as JSON over MQTT.
- Retries Wi-Fi, MQTT, and Blynk connections instead of assuming the network is always available.

> **Status:** The sketch has been updated in the repository, but it has not yet been verified by a successful compile or hardware/Wokwi test. Treat it as a work in progress.

## Repository structure

```text
VigilantPotato/
├── VigilantPotato.ino
├── LIBRARIES.md
└── secrets.h.example
```

- `VigilantPotato/VigilantPotato.ino` — main Arduino sketch.
- `VigilantPotato/LIBRARIES.md` — Arduino IDE setup and library list.
- `VigilantPotato/secrets.h.example` — example for configuring the Blynk token safely.

## Hardware and pin connections

The current sketch is configured for these ESP32 pins:

| Component | Signal | ESP32 pin |
|---|---|---|
| DHT22 | DATA | GPIO 4 |
| DHT22 | VCC | 3V3 |
| DHT22 | GND | GND |
| I²C OLED | SDA | GPIO 21 |
| I²C OLED | SCL | GPIO 22 |
| OLED | VCC/GND | Follow the exact display board's voltage requirements and connect GND |

The sketch currently uses the Adafruit SH110X library and an SH1107-style 128×128 OLED at I²C address `0x3C`. Your earlier Grove SH1107 display may differ by board revision. Confirm its controller, resolution, address, and voltage. If your Wokwi display is an SSD1306 128×64 model, the display library and dimensions must be changed to match the simulated component.

## Requirements

- Arduino IDE 2
- ESP32 board support by Espressif Systems
- Libraries: Blynk, DHT sensor library, Adafruit GFX Library, Adafruit SH110X, and PubSubClient

See [LIBRARIES.md](VigilantPotato/LIBRARIES.md) for setup details.

## Configure Blynk credentials

1. Copy `VigilantPotato/secrets.h.example` to a new file named `VigilantPotato/secrets.h`.
2. Replace the placeholder token in `secrets.h` with your current Blynk device authentication token.
3. Keep `secrets.h` private and **do not commit it to GitHub**.

The sketch uses the following Blynk virtual pins:

- **V0** — temperature in °C
- **V1** — relative humidity in %

In the Blynk dashboard, create matching datastreams and widgets for V0 and V1. Make sure the template ID/name and device token correspond to your own Blynk device.

## Wi-Fi settings

The sketch currently uses the Wokwi simulator's guest network:

- SSID: `Wokwi-GUEST`
- Password: empty

For physical hardware, replace these with your local Wi-Fi credentials. Do not publish private Wi-Fi credentials in a public repository.

## MQTT settings

The demo sketch uses:

- Broker: `test.mosquitto.org`
- Port: `1883` (unencrypted)
- Topic: `vigilant-potato/esp32/telemetry`

Example payload:

```json
{"temperature":27.5,"humidity":61.2}
```

This is a public demonstration broker, not a private or secure production service. Use a unique topic and never send sensitive information through it.

## Build and test

1. Install Arduino IDE 2 and ESP32 board support.
2. Install all required libraries listed above.
3. Copy and configure `secrets.h` locally.
4. Open `VigilantPotato/VigilantPotato.ino` in Arduino IDE.
5. Select the correct ESP32 board and port (for a physical board).
6. Click **Verify** before uploading.
7. Open Serial Monitor at **115200 baud** and check the Wi-Fi, Blynk, MQTT, DHT22, and OLED messages.

Test the sensor and display first, then Wi-Fi, then MQTT, and finally Blynk. A successful MQTT connection does not prove that Blynk is connected; they are separate services.

## Security note

A Blynk authentication token was included in a message shared during development. For safety, regenerate that device token in Blynk before using the project, and keep the replacement token only in your local, untracked `secrets.h` file.
