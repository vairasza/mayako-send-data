#include <Arduino.h>
#include <vector>

#include "CommandManager.h"
#include "NetworkCommands.h"
#include "DeviceManagerCommands.h"
#include "Config.h"
#include "Logger.h"
#include "NetworkManager.h"
#include "DeviceManager.h"
#include "Definitions.h"

CommandManager::CommandManager(DeviceManager &deviceManager, NetworkManager &networkManager):deviceManager(deviceManager), networkManager(networkManager) {
    
    this->logger = Logger::getInstance();
    /**
    standard commands
    */
    //device
    this->addCommand(CMD::RECORD_CREATE, new CapabilitiesCreate(this->deviceManager));
    this->addCommand(CMD::RECORD_START, new RecordStart(this->deviceManager));
    this->addCommand(CMD::RECORD_STOP, new RecordStop(this->deviceManager));
    this->addCommand(CMD::RECORD_READ, new CapabilitiesRead(this->deviceManager));
    this->addCommand(CMD::RESTART, new Restart(this->deviceManager));
    this->addCommand(CMD::BATTERY_READ, new BatteryRead(this->deviceManager));
    this->addCommand(CMD::IDENTIFY, new Identify(this->deviceManager));
    //network
    this->addCommand(CMD::CONNECTION_READ, new ConnectionRead(this->networkManager));
    this->addCommand(CMD::ACKNOWLEDGEMENT_ENABLE, new AcknowledgmentEnable(this->networkManager));
    this->addCommand(CMD::ACKNOWLEDGEMENT_DISABLE, new AcknowledgmentDisable(this->networkManager));
    
    #if WIRELESS_MODE == BLE
    #elif WIRELESS_MODE == WIFI
    this->addCommand(CMD::WIFI_PROFILE_CREATE, new WifiProfileCreate(this->networkManager));
    this->addCommand(CMD::WIFI_PROFILE_READ, new WifiProfileRead(this->networkManager));
    this->addCommand(CMD::WIFI_PROFILE_ACTIVE_READ, new WifiProfileActiveRead(this->networkManager));
    this->addCommand(CMD::WIFI_PROFILE_ALL_READ, new WifiProfileAllRead(this->networkManager));
    this->addCommand(CMD::WIFI_PROFILE_ACTIVE_SELECT, new WifiProfileActiveSelect(this->networkManager));
    this->addCommand(CMD::WIFI_PROFILE_DELETE, new WifiProfileDestroy(this->networkManager));
    #endif
}

CommandManager::~CommandManager() {
    this->commands.clear();
}

void CommandManager::addCommand(String identifier, CommandBase *cmd) {
    /*
    may add extra commands or commands from actuators
    */
    this->commands[identifier] = cmd;
}

void CommandManager::addCommand(String identifier, ActuatorBase *actuator) {
    this->actuators[identifier] = actuator;
}

void CommandManager::executeCommand(JsonDocument *json) {
    auto commandData = *json;
    String commandName = commandData["cmd_name"].as<String>();
    
    //here we check for standard commands
    if (this->commands.find(commandName) != this->commands.end()) {
        commandData.remove("cmd_name"); //remove the command name so that only the parameters are left
        this->commands[commandName]->execute(&commandData);
    }
    else if (this->actuators.find(commandName) != this->actuators.end()) {        
        this->actuators[commandName]->executeFunction(commandName, *json);        
    }
    else {
        this->logger->ferror(prefix("command %% not found"), std::vector<String>{ commandName });
    }    
}