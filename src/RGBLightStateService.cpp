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
  RGBLightStateService::updateRGBLedState(_state);
  addUpdateHandler([&](const String& originId) { onConfigUpdated(originId); }, false);
}

void RGBLightStateService::updateRGBLedState(RGBLightState& state) {
  analogWrite(state.pins.rPin, state.color.r);
  analogWrite(state.pins.gPin, state.color.g);
  analogWrite(state.pins.bPin, state.color.b);
}

void RGBLightStateService::onConfigUpdated(const String& originId) {
  Serial.print("The light's state has been updated by: ");
  Serial.println(originId);
  RGBLightStateService::updateRGBLedState(_state);
}

void RGBLightStateService::begin() {
  _fsPersistence.readFromFS();
  RGBLightStateService::updateRGBLedState(_state);
}

void RGBLightStateService::loop() {
  using namespace std::chrono;

  TimePoint currentTime = Clock::now();

  if (currentTime < lastCheckTime) {
    lastCheckTime = currentTime;
  }

  if (_state.schedules.schedules.size() == 0 || duration_cast<Seconds>(currentTime - lastCheckTime).count() < 1) {
    return;
  }

  lastCheckTime = currentTime;  // Update last check time

  bool foundActiveSchedule = false;
  for (const auto& schedule : _state.schedules.schedules) {
    if (currentTime >= schedule.start && currentTime <= schedule.end) {
      if (_state.color != schedule.color) {
        _state.color = schedule.color;
        updateRGBLedState(_state);
      }
      foundActiveSchedule = true;
      break;  // one active schedule at a time
    }
  }

  if (!foundActiveSchedule && !_state.color.isOff()) {
    Serial.println("No active schedule found");
    updateRGBLedState(_state);
  }
}
