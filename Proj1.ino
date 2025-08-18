#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <EEPROM.h>
#include "site.hpp"
#include "calibrate.hpp"

// --- Піни ---
#define DHTPIN 4
#define DHTTYPE DHT22

#define GROUND0_PIN 35
#define GROUND1_PIN 32
#define GROUND2_PIN 33
#define CALIBRATION_PIN 33 // <- Калібрування через 33-й пін

#define TDS_PIN 34

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

float tdsValue = 0.0;

int relayState = 0;

int soil0 = 0;
int soil1 = 0;
int soil2 = 0;

int RAW_DRY = 0;
int RAW_WET = 0;

// --- Калібрування ---
void handleCalibratePage() {
  int rawValue = analogRead(CALIBRATION_PIN);
  String html = calibratePageHtml;
  html.replace("%RAW%", String(rawValue));
  html.replace("%DRY%", String(RAW_DRY));
  html.replace("%WET%", String(RAW_WET));
  server.send(200, "text/html", html);
}

void handleSetDry() {
  RAW_DRY = analogRead(CALIBRATION_PIN);
  saveCalibration(); // <--
  server.sendHeader("Location", "/calibrate", true);
  server.send(302, "text/plain", "");
}

void handleSetWet() {
  RAW_WET = analogRead(CALIBRATION_PIN);
  saveCalibration(); // <--
  server.sendHeader("Location", "/calibrate", true);
  server.send(302, "text/plain", "");
}

// --- Інші обробники ---
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleData() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  int raw0 = analogRead(GROUND0_PIN);
  int raw1 = analogRead(GROUND1_PIN);
  int raw2 = analogRead(GROUND2_PIN);

  soil0 = constrain(map(raw0, RAW_DRY, RAW_WET, 0, 100), 0, 100);
  soil1 = constrain(map(raw1, RAW_DRY, RAW_WET, 0, 100), 0, 100);
  soil2 = constrain(map(raw2, RAW_DRY, RAW_WET, 0, 100), 0, 100);

  current_mA = ina219.getCurrent_mA();
  noWater = current_mA < 165;
  relayState = digitalRead(RELAY_PIN);

  String json = "{";
  json += "\"temp\":" + String(temperature, 1) + ",";
  json += "\"hum\":" + String(humidity, 1) + ",";
  json += "\"ground0\":" + String(soil0) + ",";
  json += "\"ground1\":" + String(soil1) + ",";
  json += "\"ground2\":" + String(soil2) + ",";
  json += "\"relay\":" + String(relayState) + ",";
  json += "\"current\":" + String(current_mA, 1) + ",";
  json += "\"noWater\":" + String(noWater ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

void saveCalibration() {
  EEPROM.writeInt(0, RAW_DRY);
  EEPROM.writeInt(4, RAW_WET);
  EEPROM.commit();
}

void loadCalibration() {
  RAW_DRY = EEPROM.readInt(0);
  RAW_WET = EEPROM.readInt(4);
}


// --- Setup ---
void setup() {
  Serial.begin(115200);
  EEPROM.begin(8); // мінімум 8 байтів для двох int
  loadCalibration();
  dht.begin();

  if (!ina219.begin()) {
    Serial.println("❌ INA219 не знайдено!");
    while (1);
  }
  ina219.setCalibration_32V_2A();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Підключення до Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\n✅ Wi-Fi OK");

  // Роутинг
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/calibrate", handleCalibratePage);
  server.on("/setDry", HTTP_POST, handleSetDry);
  server.on("/setWet", HTTP_POST, handleSetWet);
  server.begin();
  Serial.println("Вебсервер запущено");
}

// --- Loop ---
void loop() {
  server.handleClient();

  int raw0 = analogRead(GROUND0_PIN);
  int raw1 = analogRead(GROUND1_PIN);
  int raw2 = analogRead(GROUND2_PIN);

  soil0 = constrain(map(raw0, RAW_DRY, RAW_WET, 0, 100), 0, 100);
  soil1 = constrain(map(raw1, RAW_DRY, RAW_WET, 0, 100), 0, 100);
  soil2 = constrain(map(raw2, RAW_DRY, RAW_WET, 0, 100), 0, 100);
  
  float tdsVoltage = analogRead(TDS_PIN) * (3.3 / 4095.0);
  tdsValue = (133.42 * pow(tdsVoltage, 3) - 255.86 * pow(tdsVoltage, 2) + 857.39 * tdsVoltage) * 0.5;

  current_mA = ina219.getCurrent_mA();
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  noWater = current_mA < 165;


  if (soil0 < 60 || soil1 < 60 || soil2 < 60) {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = HIGH;
  } else {
    digitalWrite(RELAY_PIN, LOW);
    relayState = LOW;
  }

  Serial.print("🌡 "); Serial.print(temperature); Serial.print("°C | ");
  Serial.print("💧 "); Serial.print(humidity); Serial.print("% | ");
  Serial.print("Ґрунт0: "); Serial.print(soil0); Serial.print("% | ");
  Serial.print("Ґрунт1: "); Serial.print(soil1); Serial.print("% | ");
  Serial.print("Ґрунт2: "); Serial.print(soil2); Serial.print("% | ");
  Serial.print("⚡ "); Serial.print(current_mA); Serial.print(" мА | ");
  Serial.print("TDS: " + String(tdsValue) + " ppm | ");

  if (relayState == HIGH && current_mA < 165)
    Serial.print("💧 Вода: Немає");
  else if (relayState == LOW)
    Serial.print("💧 Реле вимкнене");
  else
    Serial.print("💧 Вода: OK");

  Serial.println();
  delay(2000);
}
