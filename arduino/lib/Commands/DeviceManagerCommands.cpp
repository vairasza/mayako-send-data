#include <ArduinoJson.h>

#include "DeviceManagerCommands.h"
#include "ModelBase.h"
#include "BoardBase.h"
#include "DeviceManager.h"

Restart::Restart(DeviceManager &deviceManager): deviceManager(deviceManager) {}

void Restart::execute(JsonDocument *json) {
    this->deviceManager.restart();
}

BatteryRead::BatteryRead(DeviceManager &deviceManager): deviceManager(deviceManager) {}

void BatteryRead::execute(JsonDocument *json) {
    this->deviceManager.getBattery();
}

Identify::Identify(DeviceManager &deviceManager): deviceManager(deviceManager) {}

void Identify::execute(JsonDocument *json) {
    this->deviceManager.identify(json);
}

CapabilitiesRead::CapabilitiesRead(DeviceManager &deviceManager): deviceManager(deviceManager) {}

void CapabilitiesRead::execute(JsonDocument *json) {
    this->deviceManager.readCapabilities();
}

CapabilitiesCreate::CapabilitiesCreate(DeviceManager &deviceManager): deviceManager(deviceManager) {}

void CapabilitiesCreate::execute(JsonDocument *json) {
    this->deviceManager.createCapabilities(json);
}

RecordStart::RecordStart(DeviceManager &deviceManager): deviceManager(deviceManager) {}

void RecordStart::execute(JsonDocument *json) {
    this->deviceManager.startRecord();
}

RecordStop::RecordStop(DeviceManager &deviceManager): deviceManager(deviceManager) {}

void RecordStop::execute(JsonDocument *json) {
    this->deviceManager.stopRecord();
}