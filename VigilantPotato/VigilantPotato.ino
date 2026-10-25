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

// Wokwi guest Wi-Fi. For physical hardware, use private local credentials.
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// Public demo broker; do not send private/sensitive data.
const char* MQTT_SERVER = "test.mosquitto.org";
const uint16_t MQTT_PORT = 1883;
const char* MQTT_TOPIC = "vigilant-potato/esp32/telemetry";

// DHT22
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// SSD1306 128x64 OLED, I2C address 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define I2C_SDA 21
#define I2C_SCL 22
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WiFiClient mqttNetwork;
PubSubClient mqtt(mqttNetwork);
BlynkTimer timer;

bool oledReady = false;
bool sensorValid = false;
float temperature = NAN;
float humidity = NAN;

unsigned long lastWiFiAttempt = 0;
unsigned long lastMQTTAttempt = 0;
unsigned long lastBlynkAttempt = 0;

const unsigned long WIFI_RETRY_MS = 10000;
const unsigned long MQTT_RETRY_MS = 10000;
const unsigned long BLYNK_RETRY_MS = 15000;

void startWiFi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lastWiFiAttempt = millis();
}

void maintainWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  if (millis() - lastWiFiAttempt >= WIFI_RETRY_MS) {
    Serial.println("Wi-Fi disconnected; retrying...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    lastWiFiAttempt = millis();
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

  if (sensorValid) {
    display.setTextSize(2);
    display.setCursor(0, 18);
    display.print("T:");
    display.print(temperature, 1);
    display.println(" C");

    display.setCursor(0, 42);
    display.print("H:");
    display.print(humidity, 1);
    display.println("%");
  } else {
    display.setCursor(0, 24);
    display.println("DHT22 read error");
    display.println("Check GPIO 4");
  }

  display.display();
}

void maintainMQTT() {
  if (WiFi.status() != WL_CONNECTED || mqtt.connected()) return;
  if (millis() - lastMQTTAttempt < MQTT_RETRY_MS) return;

  lastMQTTAttempt = millis();

  // Use the ESP32's MAC address for a stable, device-specific client ID.
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  String clientId = "VigilantPotato-" + mac;

  Serial.print("Connecting to MQTT ");
  Serial.print(MQTT_SERVER);
  Serial.print(":");
  Serial.print(MQTT_PORT);
  Serial.print(" ... ");

  if (mqtt.connect(clientId.c_str())) {
    Serial.println("CONNECTED");
    Serial.print("MQTT topic: ");
    Serial.println(MQTT_TOPIC);
  } else {
    Serial.print("FAILED, PubSubClient state = ");
    Serial.println(mqtt.state());
    Serial.println("State -2 means the TCP connection to the broker failed.");
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
    Serial.println("BLYNK CONNECTION FAILED; will retry");
  }
}

void readSensorAndPublish() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    sensorValid = false;
    Serial.println("ERROR: DHT22 read failed. Check VCC, GND, DATA GPIO 4.");
    updateOLED();
    return;
  }

  temperature = t;
  humidity = h;
  sensorValid = true;

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

  if (Blynk.connected()) {
    Blynk.virtualWrite(V0, temperature);
    Blynk.virtualWrite(V1, humidity);
    Serial.printf("BLYNK SENT: V0=%.1f C, V1=%.1f%%\n",
                  temperature, humidity);
  } else {
    Serial.println("Blynk offline; waiting for reconnect.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println("VIGILANT POTATO STARTING");

  Wire.begin(I2C_SDA, I2C_SCL);
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
    Serial.println("OLED initialized at I2C 0x3C.");
  } else {
    Serial.println("OLED initialization failed. Check SDA=21, SCL=22, address=0x3C.");
  }

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setKeepAlive(30);
  mqtt.setSocketTimeout(3);
  mqtt.setBufferSize(256);

  // Blynk uses TLS on port 443.
  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 443);

  startWiFi();

  // Permit the first broker/cloud connection attempt immediately.
  lastMQTTAttempt = millis() - MQTT_RETRY_MS;
  lastBlynkAttempt = millis() - BLYNK_RETRY_MS;

  timer.setInterval(5000L, readSensorAndPublish);

  Serial.println("Wi-Fi SSID: Wokwi-GUEST");
  Serial.println("MQTT broker: test.mosquitto.org:1883");
  Serial.println("MQTT topic: vigilant-potato/esp32/telemetry");
  Serial.println("Blynk virtual pins: V0=temperature, V1=humidity");
  Serial.println("System initialized.");
}

void loop() {
  maintainWiFi();

  if (WiFi.status() == WL_CONNECTED) {
    // Service network clients on every loop iteration.
    mqtt.loop();
    Blynk.run();

    maintainMQTT();
    maintainBlynk();
  }

  timer.run();
}
