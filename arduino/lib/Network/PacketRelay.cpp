#include <Arduino.h>
#include <ArduinoJson.h>
#include <queue>
#include <vector>
#include <memory>

#include "PacketRelay.h"
#include "Packet.h"
#include "Logger.h"
#include "Config.h"

PacketRelay *PacketRelay::instance = nullptr;

PacketRelay::PacketRelay(): packetQueue(nullptr) {}

PacketRelay *PacketRelay::getInstance() {
    if (instance == nullptr) {
        instance = new PacketRelay();
    }

    return instance;
}

void PacketRelay::setQueue(std::queue<std::shared_ptr<Packet>> *queue) {
    if (this->packetQueue == nullptr) {
        this->packetQueue = queue;
    }
}

void PacketRelay::info(char* payload) {
    if (this->packetQueue == nullptr) return;

    std::shared_ptr<Packet> packet = std::make_shared<Packet>();
    packet->setMethod(NET::HEADER::METHOD_INFO);
    packet->setPayload(payload);

    this->packetQueue->push(std::move(packet));
}

void PacketRelay::heartbeat() {
    if (this->packetQueue == nullptr) return;
    
    std::shared_ptr<Packet> packet = std::make_shared<Packet>();   
    packet->setMethod(NET::HEADER::METHOD_HEARTBEAT);
    packet->setPayload("{}");

    this->packetQueue->push(std::move(packet));
}

void PacketRelay::ack(char* payload) {
    if (this->packetQueue == nullptr) return;

    std::shared_ptr<Packet> packet = std::make_shared<Packet>();   
    packet->setMethod(NET::HEADER::METHOD_ACKNOWLEDGEMENT);
    packet->setPayload(payload);

    this->packetQueue->push(std::move(packet));
}

