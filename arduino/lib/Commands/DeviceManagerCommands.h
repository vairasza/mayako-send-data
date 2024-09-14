#ifndef DeviceManager_COMMANDS_H
#define DeviceManager_COMMANDS_H

#include <ArduinoJson.h>

#include "CommandBase.h"
#include "DeviceManager.h"
#include "Logger.h"
#include "ModelBase.h"
#include "BoardBase.h"

class Restart: public CommandBase {
    public:
        Restart(DeviceManager &deviceManager);
        void execute(JsonDocument *json) override;

    private:
        DeviceManager &deviceManager;
};

class BatteryRead: public CommandBase {
    public:
        BatteryRead(DeviceManager &deviceManager);
        void execute(JsonDocument *json) override;

    private:
        DeviceManager &deviceManager;
};

class Identify: public CommandBase {
    public:
        Identify(DeviceManager &deviceManager);
        void execute(JsonDocument *json) override;

    private:
        DeviceManager &deviceManager;
};

class CapabilitiesRead: public CommandBase {
    public:
        CapabilitiesRead(DeviceManager &deviceManager);
        void execute(JsonDocument *json) override;

    private:
        DeviceManager &deviceManager;
};

class CapabilitiesCreate: public CommandBase {
    public:
        CapabilitiesCreate(DeviceManager &deviceManager);
        void execute(JsonDocument *json) override;

    private:
        DeviceManager &deviceManager;
};

class RecordStart: public CommandBase {
    public:
        RecordStart(DeviceManager &deviceManager);
        void execute(JsonDocument *json) override;

    private:
        DeviceManager &deviceManager;
};

class RecordStop: public CommandBase {
    public:
        RecordStop(DeviceManager &deviceManager);
        void execute(JsonDocument *json) override;

    private:
        DeviceManager &deviceManager;
};

#endif