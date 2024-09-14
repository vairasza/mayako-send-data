#ifndef PACKET_H
#define PACKET_H

#include <Arduino.h>
#include <memory>

class Packet {
public:
    Packet();
    ~Packet() = default;

    void setMethod(uint8_t method);
    void setNodeIdentity(const char* nodeIdentity);
    void setSequence(uint16_t sequence);
    void setPayload(const char* payload);

    uint8_t getMethod();
    std::unique_ptr<char[]> getNodeIdentity();
    uint16_t getSequence();
    uint8_t getChecksum();
    uint16_t getPayloadSize();//does not include \0 - not in cpp with strlen and not in python
    std::unique_ptr<char[]> getPayload();

    size_t getHeaderSize();
    size_t getPacketSize();

    uint8_t* serialize();
    bool deserializeHeader(uint8_t* headerBuffer);
    bool deserializePayload(char* payloadBuffer);
    bool verifyGoodPacket();
    static bool verifyFlag(char flag);

private:
    uint8_t method;
    uint32_t nodeIdentity;
    uint16_t sequence;
    uint8_t checksum;
    uint16_t payloadSize;
    std::unique_ptr<char[]> payload;

    const size_t headerSize;
    size_t packetSize;

    void setChecksum(uint8_t checksum);
    void setPayloadSize(uint16_t payloadSize);
    uint8_t calculateChecksum(char* payload, size_t length);
    bool verifyChecksum(char* payload, size_t length);
};

#endif