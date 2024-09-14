#include <ArduinoJson.h>

#include "NetworkCommands.h"
#include "NetworkManager.h"
#include "Definitions.h"

ConnectionRead::ConnectionRead(NetworkManager &networkManager): networkManager(networkManager) {}

void ConnectionRead::execute(JsonDocument *json) {
    this->networkManager.readConnection();
}

#if WIRELESS_MODE == BLE
#elif WIRELESS_MODE == WIFI
WifiProfileCreate::WifiProfileCreate(NetworkManager &networkManager): networkManager(networkManager) {}

void WifiProfileCreate::execute(JsonDocument *json) {
    this->networkManager.createWifiProfile(json);
}

WifiProfileRead::WifiProfileRead(NetworkManager &networkManager): networkManager(networkManager) {}

void WifiProfileRead::execute(JsonDocument *json) {
    this->networkManager.readWifiProfile(json);
}

WifiProfileActiveRead::WifiProfileActiveRead(NetworkManager &networkManager): networkManager(networkManager) {}

void WifiProfileActiveRead::execute(JsonDocument *json) {
    this->networkManager.readActiveWifiProfile(json);
}

WifiProfileAllRead::WifiProfileAllRead(NetworkManager &networkManager): networkManager(networkManager) {}

void WifiProfileAllRead::execute(JsonDocument *json) {
    this->networkManager.readWifiAllProfiles(json);
}

WifiProfileActiveSelect::WifiProfileActiveSelect(NetworkManager &networkManager): networkManager(networkManager) {}

void WifiProfileActiveSelect::execute(JsonDocument *json) {
    this->networkManager.selectActiveWifiProfile(json);
}

WifiProfileDestroy::WifiProfileDestroy(NetworkManager &networkManager): networkManager(networkManager) {}

void WifiProfileDestroy::execute(JsonDocument *json) {
    this->networkManager.destroyWifiProfile(json);
}
#endif

AcknowledgmentEnable::AcknowledgmentEnable(NetworkManager &networkManager): networkManager(networkManager)  {}

void AcknowledgmentEnable::execute(JsonDocument *json) {
    this->networkManager.enableAckPackets();
}

AcknowledgmentDisable::AcknowledgmentDisable(NetworkManager &networkManager): networkManager(networkManager)  {}

void AcknowledgmentDisable::execute(JsonDocument *json) {
    this->networkManager.disableAckPackets();
}
