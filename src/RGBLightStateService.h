#ifndef TestStateService_h
#define TestStateService_h

#include <HttpEndpoint.h>
#include <FSPersistence.h>
#include <WebSocketTxRx.h>

#define DEFAULT_RED_PIN 25
#define DEFAULT_GREEN_PIN 26
#define DEFAULT_BLUE_PIN 27

#define DEFAULT_RED_STATE 0
#define DEFAULT_GREEN_STATE 0
#define DEFAULT_BLUE_STATE 0

#define RGB_LIGHT_SETTINGS_ENDPOINT_PATH "/rest/rgbLightState"
#define RGB_LIGHT_SETTINGS_SOCKET_PATH "/ws/rgbLightState"
#define RGB_LIGHT_SETTINGS_FILE "/config/rgbLightState.json"



class RGBLightState {
 public:
  int rPin, gPin, bPin;
  int rValue, gValue, bValue;

  static void read(RGBLightState& settings, JsonObject& root) {
    root["rPin"] = settings.rPin;
    root["gPin"] = settings.gPin;
    root["bPin"] = settings.bPin;

    root["rValue"] = settings.rValue;
    root["gValue"] = settings.gValue;
    root["bValue"] = settings.bValue;
  }

  static StateUpdateResult update(JsonObject& root, RGBLightState& lightState) {
    int rState = root["rValue"].as<int>();
    int gState = root["gValue"].as<int>();
    int bState = root["bValue"].as<int>();

    int rPin = root["rPin"].as<int>();
    int gPin = root["gPin"].as<int>();
    int bPin = root["bPin"].as<int>();

    if (lightState.rValue != rState || lightState.gValue != gState || lightState.bValue != bState ||
      lightState.rPin != rPin || lightState.gPin != gPin || lightState.bPin != bPin) {
      lightState.rValue = rState;
      lightState.gValue = gState;
      lightState.bValue = bState;

      lightState.rPin = rPin;
      lightState.gPin = gPin;
      lightState.bPin = bPin;
      return StateUpdateResult::CHANGED;
    }

    return StateUpdateResult::UNCHANGED;
  }
};

class RGBLightStateService : public StatefulService<RGBLightState> {
 public:
  RGBLightStateService(AsyncWebServer* server, SecurityManager* securityManager, FS* fs);
  void begin();

  void updateRGBLedState(RGBLightState& state);

 private:
  HttpEndpoint<RGBLightState> _httpEndpoint;
  WebSocketTxRx<RGBLightState> _webSocket;
  FSPersistence<RGBLightState> _fsPersistence;

  void onConfigUpdated(const String& originId);
};

#endif
