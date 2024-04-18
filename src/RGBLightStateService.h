#ifndef TestStateService_h
#define TestStateService_h

#include <HttpEndpoint.h>
#include <FSPersistence.h>
#include <WebSocketTxRx.h>
#include <type_traits>

#define DEFAULT_RED_PIN 25
#define DEFAULT_GREEN_PIN 26
#define DEFAULT_BLUE_PIN 27

#define DEFAULT_RED_STATE 0
#define DEFAULT_GREEN_STATE 0
#define DEFAULT_BLUE_STATE 0

#define RGB_LIGHT_SETTINGS_ENDPOINT_PATH "/rest/rgbLightState"
#define RGB_LIGHT_SETTINGS_SOCKET_PATH "/ws/rgbLightState"
#define RGB_LIGHT_SETTINGS_FILE "/config/rgbLightState.json"


struct RGBPins {
  int rPin, gPin, bPin;

  RGBPins(int r, int g, int b) : rPin(r), gPin(g), bPin(b) {
  }

  bool operator==(const RGBPins& other) const {
    return rPin == other.rPin && gPin == other.gPin && bPin == other.bPin;
  }
  bool operator!=(const RGBPins& other) const {
    return !(*this == other);
  }

  void setPins(int r, int g, int b) {
    rPin = r;
    gPin = g;
    bPin = b;
  }
};

struct RGBColour {
  int r, g, b;

  RGBColour(int red, int green, int blue) : r(red), g(green), b(blue) {
  }

  bool operator==(const RGBColour& other) const {
    return r == other.r && g == other.g && b == other.b;
  }
  bool operator!=(const RGBColour& other) const {
    return !(*this == other);
  }

  bool isOff() const {
    return r == 0 && g == 0 && b == 0;
  }

  void setOff() {
    r = 0;
    g = 0;
    b = 0;
  }

  void setColor(int red, int green, int blue) {
    r = red;
    g = green;
    b = blue;
  }
};

struct Schedule {
  time_t start;
  time_t end;
  RGBColour color;

  Schedule(time_t s, time_t e, RGBColour c) : start(s), end(e), color(c) {
  }

  bool operator==(const Schedule& other) const {
    return start == other.start && end == other.end && color.r == other.color.r && color.g == other.color.g &&
           color.b == other.color.b;
  }
  bool operator!=(const Schedule& other) const {
    return !(*this == other);
  }
  bool operator<(const Schedule& other) const {
    return start < other.start;
  }
  bool operator>(const Schedule& other) const {
    return start > other.start;
  }
  bool operator<=(const Schedule& other) const {
    return start <= other.start;
  }
  bool operator>=(const Schedule& other) const {
    return start >= other.start;
  }
};

class Schedules {
 public:
  std::vector<Schedule> schedules;

  static void serializeToJsonAndRead(const Schedules& schedules, JsonArray& schedulesArray) {
    for (const auto& schedule : schedules.schedules) {
      JsonObject scheduleObj = schedulesArray.createNestedObject();
      scheduleObj["start"] = static_cast<long long>(schedule.start);  // Convert time_t to long long
      scheduleObj["end"] = static_cast<long long>(schedule.end);
      JsonObject colorObj = scheduleObj.createNestedObject("color");
      colorObj["r"] = schedule.color.r;
      colorObj["g"] = schedule.color.g;
      colorObj["b"] = schedule.color.b;
    }
  }

  static StateUpdateResult deserializeJsonAndUpdate(const JsonArray& schedulesArray, Schedules& settings) {
    std::vector<Schedule> newSchedules;

    for (JsonObject scheduleObj : schedulesArray) {
      time_t start = static_cast<time_t>(strtoll(scheduleObj["start"], nullptr, 10));
      time_t end = static_cast<time_t>(strtoll(scheduleObj["end"], nullptr, 10));

      JsonObject colorObj = scheduleObj["color"];
      int r = colorObj["r"].as<int>();
      int g = colorObj["g"].as<int>();
      int b = colorObj["b"].as<int>();

      newSchedules.push_back(Schedule(start, end, RGBColour(r, g, b)));
    }

    // Compare new schedules with existing ones to determine if there's a change
    if (settings.schedules != newSchedules) {
      settings.schedules = std::move(newSchedules);
      return StateUpdateResult::CHANGED;
    }

    return StateUpdateResult::UNCHANGED;
  }

  const std::vector<Schedule>& getSchedules() const {
    return schedules;
  }
};

class RGBLightState {
 public:
  RGBPins pins;
  RGBColour color;
  Schedules schedules;

  RGBLightState() : pins(DEFAULT_RED_PIN, DEFAULT_GREEN_PIN, DEFAULT_BLUE_PIN), color(0, 0, 0) {
  }

  static void read(RGBLightState& settings, JsonObject& root) {
    root["rPin"] = settings.pins.rPin;
    root["gPin"] = settings.pins.gPin;
    root["bPin"] = settings.pins.bPin;

    root["rValue"] = settings.color.r;
    root["gValue"] = settings.color.g;
    root["bValue"] = settings.color.b;

    // Reading schedules
    JsonArray jsonSchedulesArray = root.createNestedArray("schedules");
    Schedules::serializeToJsonAndRead(settings.schedules, jsonSchedulesArray);
  }

  static StateUpdateResult update(JsonObject& root, RGBLightState& lightState) {
    bool changed = false;

    int rState = root["rValue"].as<int>();
    int gState = root["gValue"].as<int>();
    int bState = root["bValue"].as<int>();
    int rPin = root["rPin"].as<int>();
    int gPin = root["gPin"].as<int>();
    int bPin = root["bPin"].as<int>();

    // Update colors if colours changed
    if (lightState.color != RGBColour(rState, gState, bState)) {
      lightState.color.setColor(rState, gState, bState);
      changed = true;
    }

    // Update pins if necessary
    if (lightState.pins != RGBPins(rPin, gPin, bPin)) {
      lightState.pins.setPins(rPin, gPin, bPin);
      changed = true;
    }

    // Update schedules if necessary
    if (root.containsKey("schedules") && root["schedules"].is<JsonArray>()) {
      const JsonArray& schedulesArray = root["schedules"].as<JsonArray>();
      StateUpdateResult scheduleResult = Schedules::deserializeJsonAndUpdate(schedulesArray, lightState.schedules);
      if (scheduleResult == StateUpdateResult::CHANGED) {
        changed = true;
      }
    } else {
      Serial.println("No schedules found in JSON (update).");
    }

    return changed ? StateUpdateResult::CHANGED : StateUpdateResult::UNCHANGED;
  }
};

class RGBLightStateService : public StatefulService<RGBLightState> {
 public:
  RGBLightStateService(AsyncWebServer* server, SecurityManager* securityManager, FS* fs);
  void begin();
  void loop();

  void updateRGBLedState(RGBLightState& state);

 private:
  HttpEndpoint<RGBLightState> _httpEndpoint;
  WebSocketTxRx<RGBLightState> _webSocket;
  FSPersistence<RGBLightState> _fsPersistence;

  time_t lastCheckTime = 0; 

  void onConfigUpdated(const String& originId);
};

#endif