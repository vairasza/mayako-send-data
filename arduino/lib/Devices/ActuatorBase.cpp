#include "ActuatorBase.h"
#include "DeviceBase.h"
#include "Capabilities.h"
#include "Config.h"

ActuatorBase::ActuatorBase(const String &identity): DeviceBase(identity) {
    this->capabilities = ActuatorCapabilities();
    this->resetActuatorCapabilities();
}

ActuatorBase::~ActuatorBase() {}

ActuatorCapabilities* ActuatorBase::getActuatorCapabilties() {
    return &this->capabilities;
}

void ActuatorBase::setActuatorCapabilities(ActuatorCapabilities* capabilities) {
    this->capabilities.enable = capabilities->enable;
}

void ActuatorBase::resetActuatorCapabilities() {
    this->capabilities.enable = DEVICE::DEFAULT_INCLUDE;//the command functions must have an if statement which checks if the actuator is enabled and only then executes the function
}
