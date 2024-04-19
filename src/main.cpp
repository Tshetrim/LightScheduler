#include <ESP8266React.h>
#include <LightMqttSettingsService.h>
#include <LightStateService.h>
#include <time.h>

#include <RGBLightStateService.h>

#define SERIAL_BAUD_RATE 115200

AsyncWebServer server(80);
ESP8266React esp32React(&server);

RGBLightStateService rgbLightStateService =
    RGBLightStateService(&server, esp32React.getSecurityManager(), esp32React.getFS());

void setup() {
  // start serial and filesystem
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("ESP32 is up and running...");

  // start the framework and demo project
  esp32React.begin();

  // load the initial light settings
  rgbLightStateService.begin();

  // start the server
  server.begin();
}

void loop() {
  // run the framework's loop function
  esp32React.loop();
  rgbLightStateService.loop();
  // loopPrintTime();
}
