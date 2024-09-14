#ifndef SERIAL_PROTOCOL_H
#define SERIAL_PROTOCOL_H

#include <memory>

#include "ProtocolBase.h"
#include "Packet.h"
#include "Definitions.h"

class SerialProtocol: public ProtocolBase {
    public:
        SerialProtocol();
        void init() override;
        void destroy() override;
        void writePacket(std::shared_ptr<Packet> packet) override;
        std::shared_ptr<Packet> readPacket() override;
        bool checkConnection() override;
};

#endif