#include <RGBLightStateService.h>
#include <ctime>

RGBLightStateService::RGBLightStateService(AsyncWebServer* server, SecurityManager* securityManager, FS* fs) :
    _httpEndpoint(RGBLightState::read,
                  RGBLightState::update,
                  this,
                  server,
                  RGB_LIGHT_SETTINGS_ENDPOINT_PATH,
                  securityManager,
                  AuthenticationPredicates::IS_AUTHENTICATED),
    _webSocket(RGBLightState::read,
               RGBLightState::update,
               this,
               server,
               RGB_LIGHT_SETTINGS_SOCKET_PATH,
               securityManager,
               AuthenticationPredicates::IS_AUTHENTICATED),
    _fsPersistence(RGBLightState::read, RGBLightState::update, this, fs, RGB_LIGHT_SETTINGS_FILE) {
  RGBLightStateService::updateRGBLedState();
  addUpdateHandler([&](const String& originId) { onConfigUpdated(originId); }, false);
}

void RGBLightStateService::updateRGBLedState() {
  if(currentColor == _state.color) {
    return;
  }
  analogWrite(_state.pins.rPin, _state.color.r);
  analogWrite(_state.pins.gPin, _state.color.g);
  analogWrite(_state.pins.bPin, _state.color.b);
  currentColor = _state.color;
}

void RGBLightStateService::temporarilyUpdateRGBLedState(const RGBColor& newColor) {
  if(currentColor == newColor) {
    return;
  }
  analogWrite(_state.pins.rPin, newColor.r);
  analogWrite(_state.pins.gPin, newColor.g);
  analogWrite(_state.pins.bPin, newColor.b);
  currentColor = newColor;
}

void RGBLightStateService::onConfigUpdated(const String& originId) {
  Serial.print("The light's state has been updated by: ");
  Serial.println(originId);
  RGBLightStateService::updateRGBLedState();
}

void RGBLightStateService::begin() {
  _fsPersistence.readFromFS();
  RGBLightStateService::updateRGBLedState();
}

std::string getDayOfWeek(const TimePoint& timePoint) {
  auto timeT = Clock::to_time_t(timePoint);
  std::tm* ptm = std::localtime(&timeT);
  switch (ptm->tm_wday) {
    case 0:
      return "Sunday";
    case 1:
      return "Monday";
    case 2:
      return "Tuesday";
    case 3:
      return "Wednesday";
    case 4:
      return "Thursday";
    case 5:
      return "Friday";
    case 6:
      return "Saturday";
    default:
      return "";
  }
}

bool isTimeWithin24HourWindow(const TimePoint& currentTime, const TimePoint& startTime, const TimePoint& endTime) {
  using namespace std::chrono;
  auto currentSeconds = duration_cast<Seconds>(currentTime.time_since_epoch() % Seconds(86400));
  auto startSeconds = duration_cast<Seconds>(startTime.time_since_epoch() % Seconds(86400));
  auto endSeconds = duration_cast<Seconds>(endTime.time_since_epoch() % Seconds(86400));
  return currentSeconds >= startSeconds && currentSeconds <= endSeconds;
}

void RGBLightStateService::loop() {
  using namespace std::chrono;

  TimePoint currentTime = Clock::now();
  std::string currentDay = getDayOfWeek(currentTime);

  if (currentTime < lastCheckTime) {
    lastCheckTime = currentTime;
  }

  if (_state.schedules.schedules.size() == 0 || duration_cast<Seconds>(currentTime - lastCheckTime).count() < 1) {
    return;
  }

  lastCheckTime = currentTime;  // Update last check time

  bool foundActiveSchedule = false;
  for (const auto& schedule : _state.schedules.schedules) {
    bool isTodayActive = schedule.isActiveOnDay(currentDay);

    if (isTodayActive && isTimeWithin24HourWindow(currentTime, schedule.start, schedule.end)) {
      temporarilyUpdateRGBLedState(schedule.color);
      foundActiveSchedule = true;
      break;
    } else if (!isTodayActive && currentTime >= schedule.start && currentTime <= schedule.end) {
      temporarilyUpdateRGBLedState(schedule.color);
      foundActiveSchedule = true;
      break;
    }
  }

  if (!foundActiveSchedule) {
    Serial.println("No active schedule found, reverting to default color");
    updateRGBLedState();  // Use default color if set
  }
}
