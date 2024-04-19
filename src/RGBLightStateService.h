#ifndef TestStateService_h
#define TestStateService_h

#include <HttpEndpoint.h>
#include <FSPersistence.h>
#include <WebSocketTxRx.h>
#include <type_traits>
#include <chrono>

using Clock = std::chrono::system_clock;
using TimePoint = Clock::time_point;
using Seconds = std::chrono::seconds;

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

struct RGBColor {
  int r, g, b;

  RGBColor(int red = 0, int green = 0, int blue = 0) : r(red), g(green), b(blue) {
  }

  bool operator==(const RGBColor& other) const {
    return r == other.r && g == other.g && b == other.b;
  }
  bool operator!=(const RGBColor& other) const {
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
  TimePoint start;
  TimePoint end;
  RGBColor color;

  Schedule(TimePoint s = Clock::now(), TimePoint e = Clock::now() + Seconds(60), RGBColor c = RGBColor(0, 0, 0)) :
      start(s), end(e), color(c) {
  }

  bool operator==(const Schedule& other) const {
    return start == other.start && end == other.end && color == other.color;
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
      scheduleObj["start"] = std::chrono::duration_cast<Seconds>(schedule.start.time_since_epoch()).count();
      scheduleObj["end"] = std::chrono::duration_cast<Seconds>(schedule.end.time_since_epoch()).count();
      JsonObject colorObj = scheduleObj.createNestedObject("color");
      colorObj["r"] = schedule.color.r;
      colorObj["g"] = schedule.color.g;
      colorObj["b"] = schedule.color.b;
    }
  }

  static StateUpdateResult deserializeJsonAndUpdate(const JsonArray& schedulesArray, Schedules& settings) {
    std::vector<Schedule> newSchedules;

    for (JsonObject scheduleObj : schedulesArray) {
      if (!scheduleObj.containsKey("start") || !scheduleObj.containsKey("end") ||
          !scheduleObj["color"].is<JsonObject>()) {
        Serial.println("Missing schedule information");
        continue;  // Skip malformed entries
      }

      auto start_seconds = Seconds(scheduleObj["start"].as<long long>());
      auto end_seconds = Seconds(scheduleObj["end"].as<long long>());
      TimePoint start = TimePoint(start_seconds);
      TimePoint end = TimePoint(end_seconds);

      JsonObject colorObj = scheduleObj["color"];
      int r = colorObj["r"].as<int>();
      int g = colorObj["g"].as<int>();
      int b = colorObj["b"].as<int>();

      newSchedules.push_back(Schedule(start, end, RGBColor(r, g, b)));
    }

    // Compare new schedules with existing ones to determine if there's a change
    if (settings.schedules != newSchedules) {
      settings.schedules.swap(newSchedules);
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
  RGBColor color;
  Schedules schedules;

  RGBLightState() : pins(DEFAULT_RED_PIN, DEFAULT_GREEN_PIN, DEFAULT_BLUE_PIN), color(0, 0, 0) {
  }

  static void read(RGBLightState& settings, JsonObject& root) {
    JsonObject pinsJson = root.createNestedObject("pins");
    pinsJson["rPin"] = settings.pins.rPin;
    pinsJson["gPin"] = settings.pins.gPin;
    pinsJson["bPin"] = settings.pins.bPin;

    JsonObject colorJson = root.createNestedObject("color");
    colorJson["r"] = settings.color.r;
    colorJson["g"] = settings.color.g;
    colorJson["b"] = settings.color.b;

    // Reading schedules
    JsonArray jsonSchedulesArray = root.createNestedArray("schedules");
    Schedules::serializeToJsonAndRead(settings.schedules, jsonSchedulesArray);
  }

  static StateUpdateResult update(JsonObject& root, RGBLightState& lightState) {
    bool changed = false;

    Serial.println("Received JSON:");
    serializeJsonPretty(root, Serial);

    if (root.containsKey("color") && root["color"].is<JsonObject>()) {
      JsonObject colorJson = root["color"].as<JsonObject>();
      int r = colorJson.containsKey("r") ? colorJson["r"].as<int>() : 0;
      int g = colorJson.containsKey("g") ? colorJson["g"].as<int>() : 0;
      int b = colorJson.containsKey("b") ? colorJson["b"].as<int>() : 0;

      if (lightState.color != RGBColor(r, g, b)) {
        lightState.color.setColor(r, g, b);
        changed = true;
      }
    } else {
      Serial.println("No color data found in JSON (update).");
    }

    if (root.containsKey("pins") && root["pins"].is<JsonObject>()) {
      JsonObject pinsJson = root["pins"].as<JsonObject>();
      int rPin = pinsJson["rPin"].as<int>();
      int gPin = pinsJson["gPin"].as<int>();
      int bPin = pinsJson["bPin"].as<int>();

      if (lightState.pins != RGBPins(rPin, gPin, bPin)) {
        lightState.pins.setPins(rPin, gPin, bPin);
        changed = true;
      }
    } else {
      Serial.println("No pin data found in JSON (update).");
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

  TimePoint lastCheckTime = Clock::now();

  void onConfigUpdated(const String& originId);
};

#endif