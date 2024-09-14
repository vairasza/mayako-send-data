#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>

#include "ActuatorCommand.h"

void ActuatorCommand::addFunction(const String& identifier, std::function<void(JsonDocument&)> fun) {
    this->callbacks[identifier] = fun;
}

void ActuatorCommand::executeFunction(const String& identifier, JsonDocument& args) {
    if (this->callbacks.find(identifier) != this->callbacks.end()) {
        this->callbacks[identifier](args);
    } else {
        //TODO: write error message
    }
}
