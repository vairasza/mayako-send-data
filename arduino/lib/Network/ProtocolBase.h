#ifndef PROTOCOL_BASE_H
#define PROTOCOL_BASE_H

#include <Arduino.h>
#include <memory>

#include "Packet.h"

class ProtocolBase {
    public:
        ProtocolBase(String name, uint16_t bufferSize);
        virtual void init() = 0;
        virtual void destroy() = 0;
        virtual void writePacket(std::shared_ptr<Packet> packet) = 0;
        virtual std::shared_ptr<Packet> readPacket() = 0;
        virtual bool checkConnection() = 0;
        String getName();

    protected:
        const String name;
        bool connected = false;
        uint16_t bufferSize;
};

#endif