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
  time_t currentTime = time(nullptr);

  // Check the schedules only once per second
  if (_state.schedules.getSchedules().size() == 0 || currentTime - lastCheckTime < 1) {
    return;
  }

  lastCheckTime = currentTime;  // Update last check time

  // Iterate through the schedules to find any that should currently be active
  bool foundActiveSchedule = false;
  // Serial.println("Current Time:" +String(currentTime));
  for (const auto& schedule : _state.schedules.getSchedules()) {
    // Serial.println("Checking schedule: " + String(schedule.start) + " - " + String(schedule.end));
    if (currentTime >= schedule.start && currentTime <= schedule.end) {
      if (_state.color != schedule.color) {
        _state.color = schedule.color;  
        updateRGBLedState(_state);      
      }
      foundActiveSchedule = true;
      break;  // one active schedule at a time
    }
  }

  // If no active schedule is found and the light is on, you might want to turn it off or revert to a default state
  if (!foundActiveSchedule && !_state.color.isOff()) {
    Serial.println("No active schedule found, reverting to default state.");
    // _state.color = RGBColour(0, 0, 0);  // Reset to no color
    updateRGBLedState(_state);
  }
}
