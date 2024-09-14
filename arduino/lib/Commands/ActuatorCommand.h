#ifndef ACTUATOR_COMMAND_H
#define ACTUATOR_COMMAND_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>

class ActuatorCommand {
    public:
        //https://stackoverflow.com/a/28689902
        void addFunction(const String& identifier, std::function<void(JsonDocument&)> fun);
        void executeFunction(const String& identifier, JsonDocument& doc);

    protected:
        std::map<String, std::function<void(JsonDocument&)>> callbacks;
};

#endif