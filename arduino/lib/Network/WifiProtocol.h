#ifndef UDPPROTOCOL_H
#define UDPPROTOCOL_H

#include <WiFi.h>
#include <WiFiUdp.h>
#include <memory>

#include "ProtocolBase.h"
#include "WifiProfile.h"
#include "Packet.h"
#include "Definitions.h"

class WifiProtocol: public ProtocolBase, public WiFiProfile {
    public:
        WifiProtocol(Storage *storage);
        void init() override;
        void destroy() override;
        void writePacket(std::shared_ptr<Packet> packet) override;
        std::shared_ptr<Packet> readPacket() override;
        bool checkConnection() override;

    private:
        WiFiUDP udp;
};

#endif