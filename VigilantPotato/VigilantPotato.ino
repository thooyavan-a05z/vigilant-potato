#define BLYNK_TEMPLATE_ID "TMPL3QRrt5UMK"
#define BLYNK_TEMPLATE_NAME "Vigilant Potato"
#define BLYNK_AUTH_TOKEN "PASTE_YOUR_NEW_BLYNK_TOKEN_HERE"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32_SSL.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>

// Wokwi guest Wi-Fi. For a physical ESP32, use your own Wi-Fi credentials
// locally and never commit private credentials to a public repository.
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// Public demo MQTT broker; do not publish sensitive information.
const char* MQTT_SERVER = "test.mosquitto.org";
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC = "vigilant-potato/esp32/telemetry";

// DHT22 sensor
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// SSD1306 OLED, 128x64, I2C address 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WiFiClient mqttClient;
PubSubClient mqtt(mqttClient);
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
  Serial.println("Connecting to Wi-Fi...");
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
  String clientId = "VigilantPotato-" +
                    String((uint32_t)ESP.getEfuseMac(), HEX);

  Serial.print("Connecting to MQTT... ");
  if (mqtt.connect(clientId.c_str())) {
    Serial.println("MQTT CONNECTED");
  } else {
    Serial.print("MQTT CONNECTION FAILED, state = ");
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
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("VIGILANT POTATO");
  display.drawLine(0, 12, SCREEN_WIDTH - 1, 12, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 18);
  display.print("T:");
  display.print(temperature, 1);
  display.println(" C");

  display.setCursor(0, 42);
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

  Serial.printf("Temperature: %.1f C | Humidity: %.1f%%\n",
                temperature, humidity);

  updateOLED();

  char payload[128];
  snprintf(payload, sizeof(payload),
           "{\"temperature\":%.1f,\"humidity\":%.1f}",
           temperature, humidity);

  if (mqtt.connected()) {
    if (mqtt.publish(MQTT_TOPIC, payload)) {
      Serial.print("MQTT PUBLISHED: ");
      Serial.println(payload);
    } else {
      Serial.println("MQTT PUBLISH FAILED");
    }
  } else {
    Serial.println("MQTT offline; waiting for reconnect.");
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

  Serial.printf("BLYNK SENT: V0=%.1f C, V1=%.1f%%\n",
                temperature, humidity);
}

void setup() {
  Serial.begin(115200);

  // ESP32 I2C pins: SDA = GPIO 21, SCL = GPIO 22
  Wire.begin(21, 22);
  dht.begin();

  oledReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  if (oledReady) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("VIGILANT POTATO");
    display.println();
    display.println("Starting system...");
    display.display();
    Serial.println("OLED initialized.");
  } else {
    Serial.println("OLED initialization failed. Check I2C wiring/address 0x3C.");
  }

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setKeepAlive(30);
  mqtt.setSocketTimeout(2);

  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 443);

  startWiFi();

  // Make the first MQTT and Blynk connection attempts eligible immediately.
  lastMQTTAttempt = millis() - MQTT_RETRY_MS;
  lastBlynkAttempt = millis() - BLYNK_RETRY_MS;

  timer.setInterval(5000L, readSensorAndPublish);
  timer.setInterval(5000L, sendToBlynk);

  Serial.println("==============================");
  Serial.println("VIGILANT POTATO STARTED");
  Serial.println("==============================");
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
