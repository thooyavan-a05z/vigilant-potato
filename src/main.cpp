#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Starter configuration for ESP32 + DHT22 + SH1107 OLED + MQTT.
// In Wokwi, use DHT22 DATA -> GPIO 15, OLED SDA -> GPIO 21, SCL -> GPIO 22.
// Install libraries: DHTesp, PubSubClient, Adafruit GFX Library, Adafruit SH110X.

#define DHT_PIN 15
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* MQTT_HOST = "test.mosquitto.org";
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC = "vigilant-potato/esp32/temperature";

DHTesp dht;
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
Adafruit_SH1107 display = Adafruit_SH1107(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

unsigned long lastPublish = 0;
const unsigned long PUBLISH_INTERVAL_MS = 5000;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }
}

void connectMQTT() {
  while (!mqtt.connected()) {
    String clientId = "vigilant-potato-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (!mqtt.connect(clientId.c_str())) {
      delay(1500);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.setup(DHT_PIN, DHTesp::DHT22);

  Wire.begin(21, 22);
  if (!display.begin(0x3C, true)) {
    Serial.println("OLED not found; check I2C wiring/address.");
  } else {
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Vigilant Potato");
    display.println("Starting...");
    display.display();
  }

  connectWiFi();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  if (millis() - lastPublish >= PUBLISH_INTERVAL_MS) {
    lastPublish = millis();
    TempAndHumidity reading = dht.getTempAndHumidity();

    if (isnan(reading.temperature) || isnan(reading.humidity)) {
      Serial.println("Failed to read DHT22.");
      return;
    }

    char payload[100];
    snprintf(payload, sizeof(payload),
             "{\"temperature_c\":%.1f,\"humidity_pct\":%.1f}",
             reading.temperature, reading.humidity);
    mqtt.publish(MQTT_TOPIC, payload);

    Serial.println(payload);
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Vigilant Potato");
    display.println("Temp Monitor");
    display.setTextSize(2);
    display.print(reading.temperature, 1);
    display.println(" C");
    display.setTextSize(1);
    display.print("Humidity: ");
    display.print(reading.humidity, 1);
    display.println("%");
    display.println("MQTT: published");
    display.display();
  }
}