#include <Arduino.h>
#include <CRC8.h>
#include <CRC.h>
#include <memory>

#include "Packet.h"
#include "Config.h"
#include "Logger.h"

/**
 * @class Packet
 * @file Packet.cpp
 * @brief A class for creating, serialising, deserialising network packets for the mayako framework.
 *
 * This is a binary protocol, so do use implementation such as Serial.write() to transfer data.
 * we must pay special attention to the caluclation of the payloadsize because cpp uses strings with an extra terminating character (\0) while python does not. this is why we subtract 1 from the payloadsize in the serialize method and also calculate the checksum with the length subtracted by 1 so that client and this device produce the same output.
 *
 * @code for outgoing data
 * Packet packet;
 * packet.setMethod(NET::HEADER::METHOD_DATA); //defined in Config.h
 * packet.setSequence(42);
 * packet.setPayload("Hello, world!");
 * char* buffer = packet.serialize();
 *
 * => Send buffer over the network...
 *
 * @code for incoming data
 * Packet packet;
 * if (packet.deserializeHeader(buffer) && packet.deserializePayload(buffer)) {
 *      //use data and header elements with getter functions
 *      packet.getMethod();
 *      packet.getPayload();
 * }
 *
 * @cite https://mfreiholz.de/posts/network-protocol-parser/
 * @cite https://github.com/RobTillaart/CRC
 *
 */
Packet::Packet() : method(NET::HEADER::METHOD_ACKNOWLEDGEMENT), sequence(0), checksum(0), payloadSize(0), payload(nullptr), headerSize(NET::HEADER::SIZE), nodeIdentity(0) {}

void Packet::setMethod(uint8_t method)
{
    this->method = method;
}

void Packet::setNodeIdentity(const char *nodeIdentity)
{
    // zero before setting new identity
    this->nodeIdentity = 0;

    // directly write nodeIdentity into big endian (network byte order) so that we do not need an extra function to convert it
    this->nodeIdentity |= nodeIdentity[0] << 24;
    this->nodeIdentity |= nodeIdentity[1] << 16;
    this->nodeIdentity |= nodeIdentity[2] << 8;
    this->nodeIdentity |= nodeIdentity[3] << 0;
}

void Packet::setSequence(uint16_t sequence)
{
    this->sequence = sequence;
}

void Packet::setChecksum(uint8_t checksum)
{
    this->checksum = checksum;
}

void Packet::setPayloadSize(uint16_t payloadSize)
{
    this->payloadSize = payloadSize;
}

void Packet::setPayload(const char *payload)
{
    this->payloadSize = strlen(payload) + 1;
    this->payload = std::unique_ptr<char[]>(new char[payloadSize]);
    strncpy(this->payload.get(), payload, this->payloadSize); // as char* payload already has a \0 to determine that end of the char array, we dont need to add another \0
    this->payload[this->payloadSize - 1] = '\0';
    this->checksum = this->calculateChecksum(this->payload.get(), this->payloadSize);
}

uint8_t Packet::getMethod()
{
    return this->method;
}

std::unique_ptr<char[]> Packet::getNodeIdentity()
{
    auto name = std::unique_ptr<char[]>(new char[MC::MC_NAME_LENGTH + 1]);
    name[0] = this->nodeIdentity >> 24;
    name[1] = this->nodeIdentity >> 16;
    name[2] = this->nodeIdentity >> 8;
    name[3] = this->nodeIdentity >> 0;
    name[4] = '\0';

    return name;
}

uint16_t Packet::getSequence()
{
    return this->sequence;
}

uint8_t Packet::getChecksum()
{
    return this->checksum;
}

/**
 * @brief payload size is the data of the packet and has variable size
 * @return payloadSize
 */
uint16_t Packet::getPayloadSize()
{
    return this->payloadSize;
}

/**
 * @brief payload is a json string the is the end of the packet and terminated by a \0 mark. the returned char array is allocated with new and therefore requires a delete statement when no longer needed.
 * @return buffer is a char array that can be deserialised with ArduinoJson
 */
std::unique_ptr<char[]> Packet::getPayload()
{
    auto buffer = std::unique_ptr<char[]>(new char[this->payloadSize]);
    strncpy(buffer.get(), this->payload.get(), this->payloadSize);

    return buffer;
}

/**
 * @brief header size is fixed at 9bytes
 * @return header size
 */
size_t Packet::getHeaderSize()
{
    return this->headerSize;
}

/**
 * @brief buffer size indicates the size of the packet (header with 9bytes and payload with variable size)
 * @return bufferSize
 */
size_t Packet::getPacketSize()
{
    return this->packetSize;
}

/**
 * @brief Serialise the properties method, nodeIdentity, sequence, checksum, payloadSize and payload to a header and a payload char array. There is no check if the properties are all set. Properties which take more than 1 byte of memory are translated to big endian because this is the standard format on the network and best practice. The packet is starts with one of the method flags defined in Config.h and terminated by a \0 byte. Use a method to transfer binary data.
 * @return buffer with 9 bytes prefixed header and variable sized payload.
 */
uint8_t* Packet::serialize()
{
    this->packetSize = this->headerSize + this->payloadSize;
    auto buffer = new uint8_t[this->packetSize];

    /* in the network environment data is sent in big endian format per convention, although our client "knows" that we do not. we already stored values with more than 8 bit in big endian, therefore we do not require the following conversions any more. */
    // uint16_t bigEndianSequence = __htons(this->sequence);
    // uint32_t bigEndianNodeIdentity = __htonl(this->nodeIdentity);

    //we must remove -1 from the payloadSize because the payloadSize in Packet.cpp includes the \0 character. If we send it like this, the payloadSize is always off by 1 at the client side because strings in Python do not use the \0 character because string is a class there.
    uint16_t payloadSizeCache = this->payloadSize - 1;

    buffer[0] = this->method;
    buffer[1] = this->nodeIdentity >> 24;
    buffer[2] = this->nodeIdentity >> 16;
    buffer[3] = this->nodeIdentity >> 8;
    buffer[4] = this->nodeIdentity >> 0;
    buffer[5] = this->sequence >> 8;
    buffer[6] = this->sequence >> 0;
    buffer[7] = this->checksum;
    buffer[8] = payloadSizeCache >> 8;
    buffer[9] = payloadSizeCache >> 0;

    /*
    personal notes:
    copy a specified number of bytes from one to another memory address.
    arguments:
        destination: pointer to the destination memory address; it must be num sized; can use any type
        source: a pointer to the source memory address; the source memory will not be modified; can use any type
        num: the number of bytes to copy from source to destination; uses datatype size_t

    we add this->headersize to the buffer address to ensure that the initial 9 bytes - which are the already allocated bytes for the header - are not overwritten.
    */

    memcpy(buffer + this->headerSize, this->payload.get(), this->payloadSize);

    return buffer;
}

/**
 * @brief Deserialise the header array back to the header properties in Packet which then can be retrieved by using the getter methods. does not check the array. fields which use more than 1 byte are translated back to little endian. after copying the payload to the class properties, we verify the checksum and returns false if it does not match. This would require a retry ACK packet.
 * @param buffer is a uint8_t array which does not contain a new line character anymore and starts with the a defined method flag defined in Config.h. the header size is 9 bytes and the size of the payload is written in the 9th byte.
 * @return returns false if the method flag is incorrect or the checksum does not match.
 */
bool Packet::deserializeHeader(uint8_t *headerBuffer)
{
    this->method = headerBuffer[0];
    this->nodeIdentity = 0;
    this->nodeIdentity |= headerBuffer[1] << 24;
    this->nodeIdentity |= headerBuffer[2] << 16;
    this->nodeIdentity |= headerBuffer[3] << 8;
    this->nodeIdentity |= headerBuffer[4] << 0;
    // this->nodeIdentity = __ntohl(this->nodeIdentity); //bring back into little endian
    this->sequence = 0;
    this->sequence |= headerBuffer[5] << 8;
    this->sequence |= headerBuffer[6] << 0;
    // this->sequence = __ntohs(this->sequence);
    this->checksum = headerBuffer[7];
    this->payloadSize |= headerBuffer[8] << 8;
    this->payloadSize |= headerBuffer[9] << 0;
    this->packetSize = this->payloadSize + NET::HEADER::SIZE;

    return true;
}

bool Packet::deserializePayload(char *payloadBuffer)
{
    this->payloadSize = strlen(payloadBuffer) + 1; // add one for \0 terminator because strlen is implemented to count until (and stop before)
    this->payload = std::unique_ptr<char[]>(new char[this->payloadSize]);
    strncpy(this->payload.get(), payloadBuffer, this->payloadSize - 1); // as char* payload already has a \0 to determine that end of the char array, we dont need to add another \0
                                                                        //  Add the null terminator
    this->payload[this->payloadSize - 1] = '\0';

    return true;
}

/**
 * @brief calculates the checksum using CRC8-bluetooth parameters
 * 
 * the payload must be calculated without the \0 character so that calculating it on the client side with python comes to the same result as python uses no \0. therefore we subtract 1 from the lenght of the payload ot be calculated.
 * 
 * @cite https://github.com/RobTillaart/CRC
 * @cite https://crccalc.com
 * @param payload the data attached after the header terminated with a \\0 character.
 * @param length size of payload
 * @return checksum calculation in one byte size
 */
uint8_t Packet::calculateChecksum(char *payload, size_t length)
{
    return calcCRC8((uint8_t *)payload, length - 1, 0xa7, 0x00, 0x00, true, true);
}

/**
 * @brief verifies if the checksum in the header is consistant with the calculation of the checksum of the delivered payload. payload, payloadSize and checksum are already defined because the function deserialise already assigned it to the packet instance.
 * @return true if the checksum in the header matches with the checksum calucation
 *
 */
bool Packet::verifyChecksum(char *payload, size_t length)
{
    return this->calculateChecksum(payload, length) == this->checksum;
}

/**
 * @brief verifies that the first byte of a packet has one of the with this protocol associated method flags. The method flags are listed in Config.h.
 * @param flag is the first byte of a packet and must be passed as an argument because the char array is not yet assigned to the packet instance.
 * @return returns true if the first byte is a method flag defined in Config.h
 */
bool Packet::verifyFlag(char flag)
{
    switch (flag)
    {
    case NET::HEADER::METHOD_ACKNOWLEDGEMENT:
    case NET::HEADER::METHOD_COMMAND:
    case NET::HEADER::METHOD_DATA:
    case NET::HEADER::METHOD_DEBUG:
    case NET::HEADER::METHOD_HEARTBEAT:
    case NET::HEADER::METHOD_INFO:
    case NET::HEADER::METHOD_ERROR:
        return true;
    default:
        return false;
    }
}

/**
 * @brief returns true if the header starts with one of the flags defined in Config.h and passes the checksum test for the payload.
 * @return true if flag is ok and payload has same checksum as in the header
 */
bool Packet::verifyGoodPacket()
{
    return (verifyFlag(this->method) && verifyChecksum(this->payload.get(), this->payloadSize));
}
