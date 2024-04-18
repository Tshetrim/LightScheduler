#include <ESP8266React.h>
#include <LightMqttSettingsService.h>
#include <LightStateService.h>
#include <time.h>

#include <RGBLightStateService.h>

#define SERIAL_BAUD_RATE 115200

AsyncWebServer server(80);
ESP8266React esp32React(&server);

LightMqttSettingsService lightMqttSettingsService =
    LightMqttSettingsService(&server, esp32React.getFS(), esp32React.getSecurityManager());
LightStateService lightStateService =
    LightStateService(&server, esp32React.getSecurityManager(), esp32React.getMqttClient(), &lightMqttSettingsService);

RGBLightStateService rgbLightStateService =
    RGBLightStateService(&server, esp32React.getSecurityManager(), esp32React.getFS());

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");  // EX. Saturday, June 24 2017 14:05:49
}

void loopPrintTime() {
  // Print the time at regular intervals
  static unsigned long lastTimePrint = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastTimePrint > 1000) {  // Every 1 second
    printLocalTime();
    lastTimePrint = currentTime;
  }
}

void setup() {
  // start serial and filesystem

  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("ESP32 is up and running...");

  // start the framework and demo project
  esp32React.begin();

  // load the initial light settings
  lightStateService.begin();
  rgbLightStateService.begin();

  // start the light service
  lightMqttSettingsService.begin();

  esp32React.getNTPSettingsService();

  // start the server
  server.begin();
}

void loop() {
  // run the framework's loop function
  // time_t is in seconds 
  esp32React.loop();
  rgbLightStateService.loop();
  loopPrintTime();
}
