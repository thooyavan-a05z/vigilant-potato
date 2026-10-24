#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Vigilant Potato — Arduino IDE sketch
// DHT22 DATA -> GPIO 15; OLED SDA -> GPIO 21; OLED SCL -> GPIO 22.

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
Adafruit_SH1107 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

unsigned long lastPublish = 0;
const unsigned long PUBLISH_INTERVAL_MS = 5000;
bool displayReady = false;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected");
}

void connectMQTT() {
  while (!mqtt.connected()) {
    String clientId = "vigilant-potato-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    Serial.print("Connecting to MQTT...");
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, state=");
      Serial.println(mqtt.state());
      delay(2000);
    }
  }
}

void showMessage(const char* line1, const char* line2) {
  if (!displayReady) return;
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(line1);
  display.println(line2);
  display.display();
}

void setup() {
  Serial.begin(115200);
  dht.setup(DHT_PIN, DHTesp::DHT22);

  Wire.begin(21, 22);
  // SH1107 I2C address is commonly 0x3C; verify your board's documentation.
  displayReady = display.begin(0x3C, true);
  if (!displayReady) {
    Serial.println("OLED not found. Check controller, I2C address and wiring.");
  } else {
    showMessage("Vigilant Potato", "Starting...");
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
      Serial.println("DHT22 read failed. Check sensor wiring.");
      showMessage("DHT22 read failed", "Check wiring");
      return;
    }

    char payload[100];
    snprintf(payload, sizeof(payload),
             "{\"temperature_c\":%.1f,\"humidity_pct\":%.1f}",
             reading.temperature, reading.humidity);

    bool sent = mqtt.publish(MQTT_TOPIC, payload);
    Serial.print("MQTT publish ");
    Serial.println(sent ? "OK" : "FAILED");
    Serial.println(payload);

    if (displayReady) {
      display.clearDisplay();
      display.setTextColor(SH110X_WHITE);
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println("Vigilant Potato");
      display.println("Temperature");
      display.setTextSize(2);
      display.print(reading.temperature, 1);
      display.println(" C");
      display.setTextSize(1);
      display.print("Humidity: ");
      display.print(reading.humidity, 1);
      display.println("%");
      display.println(sent ? "MQTT: sent" : "MQTT: failed");
      display.display();
    }
  }
}