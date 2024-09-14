#ifndef BLUETOOTHPROTOCOL_H
#define BLUETOOTHPROTOCOL_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <queue>
#include <memory>

#include "ProtocolBase.h"
#include "Packet.h"
#include "Logger.h"
#include "Definitions.h"

class BluetoothProtocol: public ProtocolBase, public BLECharacteristicCallbacks, public BLEServerCallbacks {
    public:
        BluetoothProtocol();
        ~BluetoothProtocol();
        void init() override;
        void destroy() override;
        void writePacket(std::shared_ptr<Packet> packet) override;
        std::shared_ptr<Packet> readPacket() override;
        bool checkConnection() override;

    private:
        void cleanup();
        void onWrite(BLECharacteristic *pCharacteristic);
        void advertise();
        void onConnect(BLEServer *pServer);
        void onDisconnect(BLEServer *pServer);

        size_t maxPayloadSize;
        Logger *logger;
        BLEServer *server;
        BLEService *service;
        BLECharacteristic *characteristic;
        std::queue<std::shared_ptr<Packet>> dataCache;
};

#endif