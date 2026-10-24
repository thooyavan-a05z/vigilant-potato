# Arduino IDE library dependencies

Install these libraries using **Sketch > Include Library > Manage Libraries** (or the Library Manager sidebar):

- **Blynk** (Blynk by Volodymyr Shymanskyy)
- **DHT sensor library** by Adafruit
- **Adafruit Unified Sensor** (dependency for the DHT library)
- **Adafruit GFX Library**
- **Adafruit SSD1306**
- **PubSubClient**

Also install ESP32 board support from Boards Manager (search for **esp32** by Espressif Systems). The ESP32 core supplies `WiFi.h`, `WiFiClient.h`, and `Wire.h`.

Select an ESP32 Dev Module board for a physical ESP32, and use Serial Monitor at **115200 baud**.

The sketch uses an SSD1306 128×64 OLED at I²C address `0x3C`, with SDA on GPIO 21 and SCL on GPIO 22.
