#include "wifi_config.h"
#include <wifi_manager.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

String forecast[3];
String forecastDate[3];
int receiveIndex = 0;
int showIndex = 0;

void getWeather() {
  HTTPClient http;

  // 浜松(西部)の天気
  http.begin("https://weather.tsukumijima.net/api/forecast?city=220040");

  int httpCode = http.GET();


  if (httpCode > 0) {
    String payload = http.getString();

    JsonDocument doc;

    deserializeJson(doc, payload);

    for (int i = 0; i < 3; i++) {
      const char* date = doc["forecasts"][i]["date"];
      String telop = weatherToEnglish(doc["forecasts"][i]["telop"]);
      const char* maxTemp = doc["forecasts"][i]["temperature"]["max"]["celsius"];
      if (maxTemp == nullptr) maxTemp = "--";

      forecastDate[i] = String(date);
      forecast[i] = telop + " " + String(maxTemp) + "C";

      Serial.println(forecast[i]);

      Serial.print(date);
      Serial.print(" : ");
      Serial.print(telop);
      Serial.println(maxTemp);
      delay(50);
    }
  } else {
    Serial.println("通信失敗");
  }

  http.end();
}

String weatherToEnglish(const char* weather) {
  String w = weather;
  // if (strcmp(weather, "晴れ") == 0) return "Sunny";
  // if (strcmp(weather, "曇り") == 0) return "Cloudy";
  // if (strcmp(weather, "雨") == 0) return "Rain";
  // if (strcmp(weather, "雪") == 0) return "Snow";


  if (w.indexOf("雪") >= 0) return "Snow";
  if (w.indexOf("雨") >= 0) {
    if (w.indexOf("雷") >= 0) return "Thunder";
    return "Rain";
  };
  if (w.indexOf("曇") >= 0) return "Cloudy";
  if (w.indexOf("晴") >= 0) return "Sunny";

  return "Unknown";
}

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed");
    while (true)
      ;
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.display();

  wifiConnect(WIFI_SSID, WIFI_PASSWORD);
  getWeather();
}

unsigned long lastUpdate = 0;
const unsigned long interval = 30UL * 60 * 1000;  // 30分

void loop() {


  if (millis() - lastUpdate >= interval) {
    getWeather();
    lastUpdate = millis();
  }

  display.clearDisplay();

  display.setTextSize(2);  // 日付は小さく
  display.setCursor(0, 0);
  display.println(forecastDate[showIndex]);

  display.setTextSize(2);  // 天気は大きく
  display.setCursor(0, 18);
  display.println(forecast[showIndex]);

  display.display();
  showIndex++;
  if (showIndex >= 3) {
    showIndex = 0;
  }
  delay(1000);
}