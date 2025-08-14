/*
#define TDS_PIN A11
#define VREF 3.3 // Напруга опорного живлення ESP32

void setup() {
  Serial.begin(115200);
}

void loop() {
  int analogValue = analogRead(TDS_PIN);
  float voltage = analogValue * (VREF / 4095.0);

  // Розрахунок TDS
  float tdsValue = (133.42 * pow(voltage, 3) - 
                    255.86 * pow(voltage, 2) + 
                    857.39 * voltage) * 0.5;

  Serial.print("🔬 Напруга: ");
  Serial.print(voltage, 2);
  Serial.print(" V\t💧 TDS: ");
  Serial.print(tdsValue, 2);
  Serial.println(" ppm");

  delay(1000);
}
*/


#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include "site.hpp"  // HTML-сторінка

// --- Піни ---
#define DHTPIN 4
#define DHTTYPE DHT22

#define GROUND0_PIN 35
#define GROUND1_PIN 32
#define GROUND2_PIN 33

#define RELAY_PIN 25

// --- Wi-Fi ---
const char* ssid = "Bombardino Crocodilo";
const char* password = "69404879269";

// --- Об'єкти ---
DHT dht(DHTPIN, DHTTYPE);
Adafruit_INA219 ina219;
WebServer server(80);

// --- Змінні ---
float temperature = 0.0;
float humidity = 0.0;
float current_mA = 0.0;
bool noWater = false;
int relayState = 0;

int soil0 = 0;
int soil1 = 0;
int soil2 = 0;

// --- Обробник HTML ---
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// --- Обробник JSON ---
void handleData() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  soil0 = map(analogRead(GROUND0_PIN), 0, 4095, 100, 0);
  soil1 = map(analogRead(GROUND1_PIN), 0, 4095, 100, 0);
  soil2 = map(analogRead(GROUND2_PIN), 0, 4095, 100, 0);
  current_mA = ina219.getCurrent_mA();
  noWater = current_mA < 165;
  relayState = digitalRead(RELAY_PIN);

  String json;

  if (isnan(temperature) || isnan(humidity)) {
    json = "{\"error\": \"Failed to read from DHT22\"}";
  } else {
    json = "{";
    json += "\"temp\":" + String(temperature, 1) + ",";
    json += "\"hum\":" + String(humidity, 1) + ",";
    json += "\"ground0\":" + String(soil0) + ",";
    json += "\"ground1\":" + String(soil1) + ",";
    json += "\"ground2\":" + String(soil2) + ",";
    json += "\"relay\":" + String(relayState) + ",";
    json += "\"current\":" + String(current_mA, 1) + ",";
    json += "\"noWater\":" + String(noWater ? "true" : "false");
    json += "}";
  }

  server.send(200, "application/json", json);
}

// --- Налаштування ---
void setup() {
  Serial.begin(115200);
  dht.begin();

  if (!ina219.begin()) {
    Serial.println("❌ Помилка INA219. Перевір з'єднання!");
    while (1);
  }
  ina219.setCalibration_32V_2A();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("🌐 Підключення до Wi-Fi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Wi-Fi підключено");
    Serial.print("📶 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Wi-Fi не підключено. Продовження в офлайн режимі...");
  }

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("✅ Вебсервер запущено");
}

// --- Основний цикл ---
void loop() {
  server.handleClient();

  soil0 = map(analogRead(GROUND0_PIN), 0, 4095, 100, 0);
  soil1 = map(analogRead(GROUND1_PIN), 0, 4095, 100, 0);
  soil2 = map(analogRead(GROUND2_PIN), 0, 4095, 100, 0);

  current_mA = ina219.getCurrent_mA();
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  noWater = current_mA < 165;

  // Вивід в Serial
  Serial.print("🌡 Темп: "); Serial.print(temperature); Serial.print("°C | ");
  Serial.print("💧 Вологість: "); Serial.print(humidity); Serial.print("% | ");
  Serial.print("🌱 Ґрунт0: "); Serial.print(soil0); Serial.print("% | ");
  Serial.print("Ґрунт1: "); Serial.print(soil1); Serial.print("% | ");
  Serial.print("Ґрунт2: "); Serial.print(soil2); Serial.print("% | ");
  Serial.print("⚡️ Струм: "); Serial.print(current_mA); Serial.print(" мА | ");

  // Автоматичне керування реле
  if (soil0 < 60 || soil1 < 60 || soil2 < 60) {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = HIGH;
  } else {
    digitalWrite(RELAY_PIN, LOW);
    relayState = LOW;
  }

  // Статус води
  if (relayState == HIGH && current_mA < 165) {
    Serial.print("💧 Вода: Закінчилась");
  } else if (relayState == LOW) {
    Serial.print("💧 Реле вимкнено");
  } else {
    Serial.print("💧 Вода: OK");
  }

  Serial.println();
  delay(2000);
}
