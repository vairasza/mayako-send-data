#ifndef PACKET_RELAY_H
#define PACKET_RELAY_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <queue>
#include <vector>
#include <memory>

#include "Packet.h"

class PacketRelay {
    public:
        //https://refactoring.guru/design-patterns/singleton/cpp/example
        static PacketRelay* getInstance();

        void setQueue(std::queue<std::shared_ptr<Packet>> *queue);

        void info(char* payload);
        void heartbeat();
        void ack(char* payload);
        
    private:
        PacketRelay();// Private constructor
        
        static PacketRelay *instance; // Static instance pointer
        std::queue<std::shared_ptr<Packet>> *packetQueue;
};

#endif
