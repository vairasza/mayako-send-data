#ifndef WIFI_PROFILE_H
#define WIFI_PROFILE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <vector>

#include "Logger.h"
#include "Storage.h"

struct WiFiCredentials {
    String ssid;
    String password;
    String clientIp;
    uint16_t clientPort;
};

class WiFiProfile {
    public:
        WiFiProfile(Storage *storage);
        ~WiFiProfile();

        void processBuildProfile();

        //for networkmanager commands
        bool readActiveProfile(JsonDocument& json);
        bool createProfile(String key, char* profile);
        bool readProfile(String key, JsonDocument& json);
        bool readAllProfiles(JsonArray& json);
        bool selectActiveProfile(String key);
        bool destroyProfile(String key);

    protected:
        WiFiCredentials credentials;

        bool loadActiveProfile();
        bool hasActiveProfile;

    private:
        Storage *storage;
        Logger *logger;

        std::vector<String> getWiFiProfileKeys();
        bool setWiFiProfileKey(String key);
        bool destroyWiFiProfileKey(String key);
        bool setActiveProfileKey(String key);
        String getActiveProfileKey();

};

#endif