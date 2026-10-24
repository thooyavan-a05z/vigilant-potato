# Vigilant Potato 🌡️

ESP32 temperature and humidity monitor using **Arduino C++**, a DHT22 sensor, an I²C OLED, Wi-Fi, and MQTT.

## Current files

- `src/main.cpp` — starter ESP32 firmware.
- `platformio.ini` — PlatformIO build configuration.

## Hardware wiring

| Module | ESP32 |
|---|---|
| DHT22 VCC | 3V3 |
| DHT22 GND | GND |
| DHT22 DATA | GPIO 15 |
| OLED SDA | GPIO 21 |
| OLED SCL | GPIO 22 |
| OLED VCC | Match the OLED board's documented voltage |
| OLED GND | GND |

The code currently targets an **SH1107 128×128 I²C OLED** at address `0x3C`. Confirm your exact Grove board's voltage and I²C address before connecting real hardware.

## Build with PlatformIO

1. Install Visual Studio Code.
2. Install the **PlatformIO IDE** extension.
3. Open this repository as a PlatformIO project.
4. Let PlatformIO install the libraries listed in `platformio.ini`.
5. Connect an ESP32 board and choose **Build** first; upload only after the build succeeds.
6. Open the serial monitor at **115200 baud**.

## Wi-Fi and MQTT

The starter code uses the Wokwi guest Wi-Fi network (`Wokwi-GUEST`) and the public MQTT broker `test.mosquitto.org` on port `1883`. These are demo settings, not production security. Use a unique MQTT topic and do not send sensitive data through the public broker.

MQTT topic: `vigilant-potato/esp32/temperature`

Example payload:
```json
{"temperature_c":27.5,"humidity_pct":61.2}
```

## Important notes

- The firmware is a starter and has **not yet been verified by a successful compile or hardware test**.
- Wokwi's available OLED simulator may use a different controller from the Grove SH1107. Confirm the simulator component/controller before expecting the display to work.
- Keep Wi-Fi credentials and broker settings in mind before using a real network.
