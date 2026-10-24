#define BLYNK_TEMPLATE_ID "TMPL3QRrt5UMK"
#define BLYNK_TEMPLATE_NAME "Vigilant Potato"
#include "secrets.h"
#define BLYNK_AUTH_TOKEN BLYNK_AUTH_TOKEN_VALUE
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32_SSL.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>

// IMPORTANT: define BLYNK_AUTH_TOKEN_VALUE in a local secrets.h file.
// Do not commit your real Blynk token to GitHub.

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

const char* MQTT_SERVER = "test.mosquitto.org";
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC = "vigilant-potato/esp32/telemetry";

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Grove SH1107 is commonly a 128x128 display. Confirm the exact board.
// If your actual display is SSD1306 128x64, use Adafruit_SSD1306 instead.
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define OLED_ADDR 0x3C

Adafruit_SH1107 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

WiFiClient espClient;
PubSubClient mqtt(espClient);
BlynkTimer timer;

bool oledReady = false;
float temperature = NAN;
float humidity = NAN;

unsigned long lastWiFiAttempt = 0;
unsigned long lastMQTTAttempt = 0;
unsigned long lastBlynkAttempt = 0;

const unsigned long WIFI_RETRY_MS = 10000;
const unsigned long MQTT_RETRY_MS = 5000;
const unsigned long BLYNK_RETRY_MS = 15000;

void startWiFi() {
  Serial.println("Starting Wi-Fi connection...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lastWiFiAttempt = millis();
}

void maintainWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  if (millis() - lastWiFiAttempt >= WIFI_RETRY_MS) {
    Serial.println("Retrying Wi-Fi...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    lastWiFiAttempt = millis();
  }
}

void maintainMQTT() {
  if (WiFi.status() != WL_CONNECTED || mqtt.connected()) return;
  if (millis() - lastMQTTAttempt < MQTT_RETRY_MS) return;

  lastMQTTAttempt = millis();
  String clientId = "VigilantPotato-" + String((uint32_t)ESP.getEfuseMac(), HEX);

  Serial.print("Connecting to MQTT... ");
  if (mqtt.connect(clientId.c_str())) {
    Serial.println("CONNECTED");
  } else {
    Serial.print("FAILED, state = ");
    Serial.println(mqtt.state());
  }
}

void maintainBlynk() {
  if (WiFi.status() != WL_CONNECTED || Blynk.connected()) return;
  if (millis() - lastBlynkAttempt < BLYNK_RETRY_MS) return;

  lastBlynkAttempt = millis();
  Serial.println("Connecting to Blynk Cloud...");

  if (Blynk.connect(1000)) {
    Serial.println("BLYNK CONNECTED");
  } else {
    Serial.println("BLYNK CONNECTION FAILED");
  }
}

void updateOLED() {
  if (!oledReady) return;

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("VIGILANT POTATO");
  display.drawLine(0, 12, SCREEN_WIDTH - 1, 12, SH110X_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 24);
  display.print("T:");
  display.print(temperature, 1);
  display.println(" C");

  display.setCursor(0, 52);
  display.print("H:");
  display.print(humidity, 1);
  display.println("%");
  display.display();
}

void readSensorAndPublish() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println("ERROR: DHT22 reading failed. Check GPIO 4 and wiring.");
    return;
  }

  temperature = t;
  humidity = h;
  Serial.printf("Temperature: %.1f C | Humidity: %.1f%%\n", temperature, humidity);

  updateOLED();

  char payload[128];
  snprintf(payload, sizeof(payload),
           "{\"temperature\":%.1f,\"humidity\":%.1f}", temperature, humidity);

  if (mqtt.connected()) {
    if (mqtt.publish(MQTT_TOPIC, payload)) {
      Serial.print("MQTT PUBLISHED: ");
      Serial.println(payload);
    } else {
      Serial.println("MQTT PUBLISH FAILED");
    }
  } else {
    Serial.println("MQTT offline; will retry.");
  }
}

void sendToBlynk() {
  if (!Blynk.connected()) {
    Serial.println("Blynk offline; waiting for reconnect.");
    return;
  }
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Waiting for valid sensor readings.");
    return;
  }

  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
  Serial.printf("BLYNK SENT: V0=%.1f C, V1=%.1f%%\n", temperature, humidity);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  dht.begin();

  oledReady = display.begin(OLED_ADDR, true);
  if (oledReady) {
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Vigilant Potato");
    display.println("Connecting...");
    display.display();
  } else {
    Serial.println("OLED initialization failed: check controller/address/wiring.");
  }

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setKeepAlive(30);
  mqtt.setSocketTimeout(2);

  // Use the Blynk Cloud SSL endpoint on port 443.
  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 443);

  startWiFi();

  // First attempts happen quickly; subsequent retries are rate-limited.
  lastBlynkAttempt = millis() - BLYNK_RETRY_MS;
  lastMQTTAttempt = millis() - MQTT_RETRY_MS;

  timer.setInterval(5000L, readSensorAndPublish);
  timer.setInterval(5000L, sendToBlynk);
  Serial.println("Vigilant Potato started.");
}

void loop() {
  maintainWiFi();

  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
    mqtt.loop();
    maintainMQTT();
    maintainBlynk();
  }

  timer.run();
}
