#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <queue>
#include <vector>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <memory>

#include "Storage.h"
#include "Logger.h"
#include "ProtocolBase.h"
#include "SerialProtocol.h"
#include "Packet.h"
#include "PacketRelay.h"
#include "IntegrityMiddleware.h"
#include "Definitions.h"

#if WIRELESS_MODE == BLE
#include "BluetoothProtocol.h"
#elif WIRELESS_MODE == WIFI
#include "WifiProtocol.h"
#include "WifiProfile.h"
#endif

class NetworkManager {
    public:
        NetworkManager(Storage *storage);
        ~NetworkManager();

        /* regular functions */
        std::vector<JsonDocument> readIncomingData();//put that data into input and inform the commandmanager
        void writeOutgoingData();//write data in output queue to network;
        void addSensorDataToOutput(std::vector<std::shared_ptr<Packet>> &sensorData);
        bool isConnected();
        void upgradeProtocol();

        /* command functions*/        
        
        #if WIRELESS_MODE == BLE
        #elif WIRELESS_MODE == WIFI
        void readWifiProfile(JsonDocument *json);
        void readWifiAllProfiles(JsonDocument *json);
        void selectActiveWifiProfile(JsonDocument *json);
        void destroyWifiProfile(JsonDocument *json);
        void createWifiProfile(JsonDocument *json);
        void readActiveWifiProfile(JsonDocument *json);
        #endif

        void sendHeartbeatToClient();
        void readConnection();
        void enableAckPackets();
        void disableAckPackets();

    private:
        unsigned long lastHeartBeat;
        unsigned long upgradeProtocolTimeout;

        ProtocolBase *currentProtocol;
        #if WIRELESS_MODE == BLE
        BluetoothProtocol *bluetoothProtocol;
        #elif WIRELESS_MODE == WIFI
        WiFiProfile *wifiProfile;
        WifiProtocol *wifiProtocol;
        #endif
        SerialProtocol *serialProtocol;
        IntegrityMiddleware integrityMiddleware;
        Logger *logger;
        PacketRelay *relay;
        std::queue<std::shared_ptr<Packet>> output;

        void sendJsonDocument(JsonDocument& doc);
        bool checkTimeout(unsigned long &lastTimeout, unsigned long interval);
};

#endif