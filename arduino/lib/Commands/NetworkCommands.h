#ifndef WIFI_COMMANDS_H
#define WIFI_COMMANDS_H

#include <ArduinoJson.h>

#include "CommandBase.h"
#include "NetworkManager.h"
#include "Definitions.h"

class ConnectionRead: public CommandBase {
    public:
        ConnectionRead(NetworkManager &networkManager);
        void execute(JsonDocument *json) override;

    private:
        NetworkManager &networkManager;
};

#if WIRELESS_MODE == BLE
#elif WIRELESS_MODE == WIFI
class WifiProfileCreate: public CommandBase {//better into networkmanager?
    public:
        WifiProfileCreate(NetworkManager &networkManager);
        void execute(JsonDocument *json) override;

    private:
        NetworkManager &networkManager;
};

class WifiProfileRead: public CommandBase {
    public:
        WifiProfileRead(NetworkManager &networkManager);
        void execute(JsonDocument *json) override;

    private:
        NetworkManager &networkManager;
};

class WifiProfileActiveRead: public CommandBase {
    public:
        WifiProfileActiveRead(NetworkManager &networkManager);
        void execute(JsonDocument *json) override;

    private:
        NetworkManager &networkManager;
};

class WifiProfileAllRead: public CommandBase {
    public:
        WifiProfileAllRead(NetworkManager &networkManager);
        void execute(JsonDocument *json) override;

    private:
        NetworkManager &networkManager;
};

class WifiProfileActiveSelect: public CommandBase {
    public:
        WifiProfileActiveSelect(NetworkManager &networkManager);
        void execute(JsonDocument *json) override;

    private:
        NetworkManager &networkManager;
};

class WifiProfileDestroy: public CommandBase {
    public:
        WifiProfileDestroy(NetworkManager &networkManager);
        void execute(JsonDocument *json) override;

    private:
        NetworkManager &networkManager;
};
#endif

class AcknowledgmentEnable: public CommandBase {
    public:
        AcknowledgmentEnable(NetworkManager &networkManager);
        void execute(JsonDocument *json) override;

    private:
        NetworkManager &networkManager;
};

class AcknowledgmentDisable: public CommandBase {
    public:
        AcknowledgmentDisable(NetworkManager &networkManager);
        void execute(JsonDocument *json) override;

    private:
        NetworkManager &networkManager;
};

#endif