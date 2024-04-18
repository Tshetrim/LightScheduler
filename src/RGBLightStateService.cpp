#include <RGBLightStateService.h>

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
  analogWrite(state.rPin, state.rValue);
  analogWrite(state.gPin, state.gValue);
  analogWrite(state.bPin, state.bValue);
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
