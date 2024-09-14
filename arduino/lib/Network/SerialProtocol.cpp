#include <Arduino.h>
#include <memory>

#include "SerialProtocol.h"
#include "Config.h"
#include "Logger.h"
#include "Packet.h"
#include "Definitions.h"

SerialProtocol::SerialProtocol() : ProtocolBase(NET::SERIAL_NAME, NET::MAX_BUFFER_SIZE) {}

void SerialProtocol::init()
{
    Serial.begin(BAUDRATE);
    this->connected = true;
}

void SerialProtocol::destroy()
{
    Serial.end();
    this->connected = false;
}

/**
 * @note we add -1 to the dataSize because we do not want to send the packet with the terminating \x00 byte. we can not add it before because we need to consider the terminating char while handling it in the packet class.
 */
void SerialProtocol::writePacket(std::shared_ptr<Packet> packet)
{
    if (!this->connected)
        return;

    uint8_t *data = packet->serialize();
    size_t dataSize = packet->getPacketSize() - 1;

    Serial.write(data, dataSize);
    
    delete[] data;
}

/**
 * @brief Read data from serial port when available. The expected packet starts with a method flag (one byte) defined in Config.h. Therefore we read bytes from serial until we recognise one of the flags. Then we create the header array with size 9. At index 8, we can read the size of the payload directly following the header.
 */
std::shared_ptr<Packet> SerialProtocol::readPacket()
{
    if (!this->connected)
        return nullptr;
    unsigned long startTime = millis();

    while (Serial.available() > 0)
    {
        // timeout requesting data from serial to avoid unresponsive microcontroller
        if (millis() - startTime >= NET::TIMEOUT_DEFAULT)
            return nullptr;

        uint8_t flag = Serial.peek();

        // Check if the read byte is the start flag
        if (Packet::verifyFlag(flag))
        {
            // Create the header array and set the first byte to the flag
            uint8_t header[NET::HEADER::SIZE];

            if (Serial.available() < NET::HEADER::SIZE)
                return nullptr;
            // Read the rest of the header
            Serial.readBytes(header, NET::HEADER::SIZE);
            // Determine the payload size from the header
            std::shared_ptr<Packet> packet = std::make_shared<Packet>();
            packet->deserializeHeader(header);

            uint16_t payloadSize = packet->getPayloadSize();

            if (Serial.available() < payloadSize)
                return nullptr;

            // Allocate the payload buffer and read the payload
            char payload[payloadSize + 1];
            Serial.readBytes(payload, payloadSize);
            payload[payloadSize] = '\0'; // Null-terminate the payload

            packet->deserializePayload(payload);

            return std::move(packet);
        }
    }

    return nullptr; // no data is available
}

bool SerialProtocol::checkConnection()
{
    if (Serial)
    {
        this->connected = true;
    }
    else
    {
        this->connected = false;
    }

    return this->connected;
}