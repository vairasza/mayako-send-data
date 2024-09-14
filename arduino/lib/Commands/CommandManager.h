#ifndef COMMAND_INVOKER_H
#define COMMAND_INVOKER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>

#include "CommandBase.h"
#include "Logger.h"
#include "NetworkManager.h"
#include "DeviceManager.h"
#include "ActuatorBase.h"
#include "Definitions.h"

class CommandManager {
    public:
        CommandManager(DeviceManager &deviceManager, NetworkManager &networkManager);
        ~CommandManager();
        void addCommand(String identifier, CommandBase* cmd);
        void addCommand(String identifier, ActuatorBase *actuator);
        void executeCommand(JsonDocument *json);

    private:
        std::map<String, CommandBase*> commands;
        std::map<String, ActuatorBase*> actuators;
        DeviceManager &deviceManager;
        NetworkManager &networkManager;
        Logger *logger;
};

#endif