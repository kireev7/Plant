#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <EEPROM.h>

#include "site.hpp"
#include "calibrate.hpp"
#include "wifi_scan.hpp"

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
const char* ap_ssid = "ESP32_Sensor_AP"; // SSID для точки доступу

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

char stored_ssid[32] = "";
char stored_password[64] = "";

// --- Функції для роботи з EEPROM ---
void saveWiFiCredentials() {
  for (int i = 0; i < 32; i++) {
    EEPROM.writeByte(8 + i, stored_ssid[i]);
  }
  for (int i = 0; i < 64; i++) {
    EEPROM.writeByte(40 + i, stored_password[i]);
  }
  EEPROM.commit();
}

void loadWiFiCredentials() {
  for (int i = 0; i < 32; i++) {
    stored_ssid[i] = EEPROM.readByte(8 + i);
  }
  for (int i = 0; i < 64; i++) {
    stored_password[i] = EEPROM.readByte(40 + i);
  }
}

void clearWiFiCredentials() {
  for (int i = 0; i < 32; i++) {
    stored_ssid[i] = '\0';
    EEPROM.writeByte(8 + i, 0);
  }
  for (int i = 0; i < 64; i++) {
    stored_password[i] = '\0';
    EEPROM.writeByte(40 + i, 0);
  }
  EEPROM.commit();
}

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
  saveCalibration();
  server.sendHeader("Location", "/calibrate", true);
  server.send(302, "text/plain", "");
}

void handleSetWet() {
  RAW_WET = analogRead(CALIBRATION_PIN);
  saveCalibration();
  server.sendHeader("Location", "/calibrate", true);
  server.send(302, "text/plain", "");
}

// --- Обробка Wi-Fi налаштувань ---
void handleWiFiPage() {
  std::vector<String> networks = scanWifiNetworks();
  String html = wifiScanPageHtml;
  String options = generateSelectOptions(networks);
  html.replace("%SELECT_OPTIONS%", options);
  server.send(200, "text/html", html);
}

void handleSetWiFi() {
  if (server.hasArg("ssid") && server.arg("ssid") != "") {
    strncpy(stored_ssid, server.arg("ssid").c_str(), 32);
    strncpy(stored_password, server.arg("password").c_str(), 64);
    saveWiFiCredentials();
    server.send(200, "text/html", "<h1>Wi-Fi data saved. Rebooting...</h1>");
    delay(1000);
    ESP.restart(); // Перезавантаження для підключення до нової мережі
  } else {
    server.send(400, "text/html", "<h1>Error: Select wi-fi</h1>");
  }
}

void handleResetWiFi() {
  clearWiFiCredentials();
  server.send(200, "text/html", "<h1>Wi-Fi data reset. Rebooting...</h1>");
  delay(1000);
  ESP.restart(); // Перезавантаження для повернення до точки доступу
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
  json += "\"noWater\":" + String(noWater ? "true" : "false") + ",";
  json += "\"tds\":" + String(tdsValue, 1);
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
  EEPROM.begin(104); // 8 байт для калібрування + 32 для SSID + 64 для пароля
  loadCalibration();
  loadWiFiCredentials();
  dht.begin();

  if (!ina219.begin()) {
    Serial.println("❌ INA219 не знайдено!");
    while (1);
  }
  ina219.setCalibration_32V_2A();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // Спроба підключення до збереженої Wi-Fi мережі
  bool wifiConnected = false;
  if (stored_ssid[0] != '\0') {
    Serial.print("Спроба підключення до Wi-Fi: ");
    Serial.println(stored_ssid);

    // Налаштування статичної IP-адреси
    IPAddress local_IP(192, 168, 0, 104); // Фіксована IP-адреса
    IPAddress gateway(192, 168, 0, 1);    // Шлюз (зазвичай IP роутера)
    IPAddress subnet(255, 255, 255, 0);   // Маска підмережі
    IPAddress primaryDNS(8, 8, 8, 8);     // DNS (наприклад, Google DNS)
    IPAddress secondaryDNS(8, 8, 4, 4);   // Додатковий DNS

    // Налаштування статичної IP
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("❌ Не вдалося налаштувати статичну IP");
    }

    WiFi.begin(stored_ssid, stored_password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      Serial.println("\n✅ Wi-Fi OK");
      Serial.print("IP-адреса: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\n❌ Не вдалося підключитися до Wi-Fi");
    }
  }

  // Якщо підключення до Wi-Fi не вдалося, створюємо точку доступу
  if (!wifiConnected) {
    WiFi.softAPConfig(IPAddress(192, 168, 0, 1), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP(ap_ssid); // Точка доступу без пароля
    Serial.println("Точка доступу створена");
    Serial.print("IP-адреса: ");
    Serial.println(WiFi.softAPIP());
  }

  // Роутинг
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/calibrate", handleCalibratePage);
  server.on("/setDry", HTTP_POST, handleSetDry);
  server.on("/setWet", HTTP_POST, handleSetWet);
  server.on("/wifi", handleWiFiPage);
  server.on("/setWifi", HTTP_POST, handleSetWiFi);
  server.on("/resetWifi", HTTP_GET, handleResetWiFi);
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