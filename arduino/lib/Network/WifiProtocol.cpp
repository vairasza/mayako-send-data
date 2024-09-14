#include <Arduino.h>
#include <vector>
#include <memory>

#include "WifiProtocol.h"
#include "Config.h"
#include "Storage.h"
#include "Packet.h"
#include "Definitions.h"

WifiProtocol::WifiProtocol(Storage *storage) : ProtocolBase(NET::WIFI_NAME, NET::MAX_BUFFER_SIZE), WiFiProfile(storage) {}

/**
 * @brief first loads the active wifi profile then create a wifi module in STA mode. We also implement onEvent with callback functions which handle the property connected
 * @cite https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiClientEvents/WiFiClientEvents.ino
 */
void WifiProtocol::init()
{
    if (this->loadActiveProfile())
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin(this->credentials.ssid, this->credentials.password);

        WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info)
                     { this->connected = true; },
                     WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
        WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info)
                     { this->connected = false; },
                     WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);

        delay(NET::TIME_TO_CONNECT_PROTOCOL);
    }
    // TODO: logger for the case that there is no connection
}

void WifiProtocol::destroy()
{
    WiFi.disconnect(true);
    // connected is handled by the event callbacks
}

/**
 * @brief writes a packet with wifi udp. after checking the connection status, we create a udp header with the clients credentials. We write the data as uint8_t because the header is in binary and sometimes contains 0x00 values in the sequence/payloadSize/checksum field which would cut the string because 0x00 cuts a C-String.
 * @note we add -1 to the dataSize because we do not want to send the packet with the terminating \x00 byte. we can not add it before because we need to consider the terminating char while handling it in the packet class.
 */
void WifiProtocol::writePacket(std::shared_ptr<Packet> packet)
{
    if (!this->connected)
        return;

    if (!this->hasActiveProfile)
        return;

    this->udp.beginPacket(this->credentials.clientIp.c_str(), this->credentials.clientPort);

    uint8_t *data = packet->serialize();
    size_t dataSize = packet->getPacketSize() - 1;
    this->udp.write(data, dataSize);
    this->udp.endPacket();
    
    delete[] data;
}

/**
 * @brief in the first step we check if there is a connection. we implement a timer which timeouts the whileloop - the value for it can be found in Config.h. Next we check if udp has data in the buffer. We start be checking the first byte for the method flag. as long as it is not the method flag, we drop the byte and look at the next byte. if we found the correct method flag (can also be foundin config.h) we check if the bytes available are bigger than our headerSize (can be found in config.h). we read the header into a integer array as especially look at byte 9 and 10 because they make a uint16_t integer telling the size of the payload which directly follows the 10th byte of the header. there we also check if the udp buffer has enough data to read the payload. after that we deserliased it with the packet class and return it.
 * @return a packet that deserialised the packet from the network; can be nullptr
 */
std::shared_ptr<Packet> WifiProtocol::readPacket()
{
    if (!this->connected)
        return nullptr;

    unsigned long startTime = millis();
    size_t packetSize = this->udp.parsePacket();

    while (this->udp.available() > 0)
    {
        if (millis() - startTime >= NET::TIMEOUT_DEFAULT)
            return nullptr;

        uint8_t flag = this->udp.peek();
        if (!Packet::verifyFlag(flag))
        {
            this->udp.read(); // discard the bytes that are not yet the flag of our packet
        }
        else
        {
            if (this->udp.available() < NET::HEADER::SIZE)
                return nullptr;

            uint8_t header[NET::HEADER::SIZE];

            this->udp.readBytes(header, NET::HEADER::SIZE);

            std::shared_ptr<Packet> packet = std::make_shared<Packet>();
            packet->deserializeHeader(header);
            uint16_t payloadSize = packet->getPayloadSize();

            if (this->udp.available() < payloadSize)
                return nullptr;

            char payload[payloadSize + 1];
            this->udp.readBytes(payload, payloadSize);
            payload[payloadSize] = '\0';

            packet->deserializePayload(payload);

            return std::move(packet);
        }
    }

    return nullptr;
}

/**
 * @brief checks if the WiFi module has a "connection" with the WiFi router. this function does not indicate if there is a client or the credentials for the client ip or port are correct and if the client is actively accepting packets.
 * @return if there is a connection
 */
bool WifiProtocol::checkConnection()
{
    return WiFi.status() == WL_CONNECTED || this->connected;
}
