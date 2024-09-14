#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <memory>
#include <math.h>

#include "IntegrityMiddleware.h"
#include "Packet.h"
#include "Config.h"
#include "Logger.h"
#include "PacketRelay.h"
#include "Definitions.h"

IntegrityMiddleware::IntegrityMiddleware(): outgoingSequence(0), expectedSequence(0), ackSendEnabled(NET::SEND_ACK_PACKETS) {
    this->logger = Logger::getInstance();
    this->relay = PacketRelay::getInstance();
}

IntegrityMiddleware::~IntegrityMiddleware() {
    this->outOfOrderPackets.clear();
    this->packetsForResend.clear();
}

/**
 * @brief Processes the read packet from the currently running protocol for the checksum/integrity, if the packets are in order or if they are corrputed.
 * 
 * This function is a middleware between the networkmanager and the currently running protocol. It is called by the networkmanager with each iteration. This function then lets the currently running protocol read a sequence of bytes until the newline character.
 * 
 * @param packet was read and deserialised in the networkprocotol and contains a request from mayako-core
 * 
 * @return output has 0+ packets that are ordered
 */
std::vector<std::shared_ptr<Packet>> IntegrityMiddleware::processIncomingData(std::shared_ptr<Packet> packet) {
    std::vector<std::shared_ptr<Packet>> output;

    //just return the vector with the incoming packet when the user does not wish to have order/checksum check for the packets
    if (!this->ackSendEnabled) {
        output.push_back(packet);
        return output;
    }

    //we got a bad packet that either has a wrong checksum, has a wrong flag (start byte) or has no data, therefore we "drop" the packet and request a resend with
    if (!packet->verifyGoodPacket()) {

        this->sendAckPacket(packet->getSequence(), true);

        return output;//empty
    } else if (packet->getMethod() == NET::HEADER::METHOD_ACKNOWLEDGEMENT) {
        //we must call this else if statement for compareSequenceWithOverflow because ACK packets do not have a sequence (they are not tracked). Otherwise we would face unexpected behaviour.
        this->processAcknowledgement(packet);

        return output; //returning since we do not progress the data any further; empty
    } else if (packet->getMethod() != NET::HEADER::METHOD_HEARTBEAT) {
        //already ruled out ACK method too
        //send ACK to client that the packet is ok
        this->sendAckPacket(packet->getSequence(), false);

        //we do return here early for heartbeat packet because we want to know if the mayako-core is still alive but not make any sequence tracking
        output.push_back(packet);
        return output;
    } else if (this->compareSequenceWithOverflow(packet->getSequence(), this->expectedSequence)) {
        //the received packet is below the sequence that we expect. it also is not a packet that we need for reordering because we do not increment the expcetedSequence counter if there is a packet with a too high sequence. Packets with "too high sequence" are saved in outOfOrderPackets and are only used when the order can be zipped. Only then will the sequence be increased by the count of "jumps" we did with filling the order gap. It might be a corrupted packet, a double packet or a packet that we "jumped over" because of too many tries or overflow (we only track back packets as long as the resendPacket map is not too big, because then we step-by-step "jump over" the gaps and forget about the missing packets).
        return output;//empty

    }
    /** packet from here on:
    - they are not corrupted
    - they are not ACK packets
    - we sent ACK if it is a good packet
    */
    this->processValidPacket(packet, output);
    
    return output;
}

void IntegrityMiddleware::processValidPacket(std::shared_ptr<Packet> packet, std::vector<std::shared_ptr<Packet>> &output) {
    if (packet->getSequence() == this->expectedSequence) {
        /**
         * the current packet has the sequence that we expect therefore we mark it as output, but also check the other packets in outOfOrderPackets. here we put the current packet in the output vector because we are sure that it is not corrupted and in order.
         */
        output.push_back(packet);
        this->processOutOfOrderPackets(output);

    } else if (this->outOfOrderPackets.size() >= NET::OUT_OF_ORDER_PACKET_MAX_SIZE) {
        //here we look at outOfOrderPackets: if the size is higher then a set maximum, we "jump over the hole" in the sequence and take the packet with the next nearest sequence. We do this with only one paket, to keep the loss low.
        this->handlePacketOverflow(output);
        this->processOutOfOrderPackets(output);
        //TODO: would be good to zip here again. right?
    } else {
        //that packet we received is above the sequence we expected, therefore we are missing packets. first, we send an ACK retry packet for the packet sequences that are between the expectedSequence and the current packet sequence. then we save the current packet in outOfOrderPackets for later retrieval.
        this->requestMissingPackets(packet);
    }
}

void IntegrityMiddleware::processOutOfOrderPackets(std::vector<std::shared_ptr<Packet>> &output) {
    //TODO while true loop could be problematic if things go bad
    while (true) {
        //we increment the expectedsequence and look in the map outOfOrderPackets if they are there. we look as long as we can find packets then break. we also pay attention to overflow in uint16_t
        this->expectedSequence = this->incrementSequence(this->expectedSequence);
        auto it = this->outOfOrderPackets.find(this->expectedSequence);
        if (it != this->outOfOrderPackets.end()) {
            std::shared_ptr<Packet> packet = it->second;
            
            output.push_back(packet);

            this->outOfOrderPackets.erase(it);
        } else {
            break;
        }
    }
}

void IntegrityMiddleware::handlePacketOverflow(std::vector<std::shared_ptr<Packet>> &output) {
    //TODO while true loop could be problematic if things go bad
    //we realised that the recevied packet does not have the desired sequence and the map outOfOrderPackets has exceeded the limit we set. so we decide to "jump over the gap". we increase the sequence and check the map as long as we can not find a "next" packet. the packets in the gap are lost and no longer tracked.
    uint16_t currentExpectedSequence = this->incrementSequence(this->expectedSequence);
    while (true) {
        auto it = this->outOfOrderPackets.find(currentExpectedSequence);
        if (it != this->outOfOrderPackets.end()) {
            std::shared_ptr<Packet> packet = it->second;
            
            output.push_back(packet);
            this->outOfOrderPackets.erase(it);
            
            break;
        } else {
            //at this sequence, we could not find a packet so we look at the next seqeunce.
            currentExpectedSequence = this->incrementSequence(currentExpectedSequence);
        }
    }
}

void IntegrityMiddleware::requestMissingPackets(std::shared_ptr<Packet> packet) {
    uint16_t sequenceCounter = this->expectedSequence;
    //here we must be cautious because the sequence can overflow if we have many packets to send and many sensors. sequence overflows after 65536 packets. is this safe then?
    //the received packet is valid but is higher than expected. therefore we send ACK packets for the packets between the expectedSequence and the sequence of the packet that we recvied instead.
    while (sequenceCounter != packet->getSequence()) {
        this->sendAckPacket(sequenceCounter, true);
        sequenceCounter = this->incrementSequence(sequenceCounter);
    }
    this->outOfOrderPackets[packet->getSequence()] = packet;
}

/**
 * @brief compare the incoming and expected sequence considering uint16_t overflow
 * @cite https://www.rfc-editor.org/rfc/rfc1982#section-3.2
 * @param i1 is the incomiongSequence
 * @param i2 is the expectedSequence
 * @return true if the incoming sequence is below in sequence
 * @example if we expect a packet with the sequence 77 (expectedSequence) but receive a packet with sequence 67, we can certainly say that it is a double, corrupt BUT NOT out of order because we ONLy increment the expectedSequence if the order is intact. These kinds of packets can be savely dropped.
 */
bool IntegrityMiddleware::compareSequenceWithOverflow(uint16_t i1, uint16_t i2) {
    if (i1 == i2) return false; //we want to make sure that equal packets are passed further
    bool condition = (i1 < i2) && ((i2 - i1) < NET::SEQUENCE_MAX_NUMBER_SIZE);
    return condition;
}

/**
 * @brief increment the sequence while keeping an eye on overflow
 * @param sequence to increment
 * @return the incremented value for sequence
 */
uint16_t IntegrityMiddleware::incrementSequence(uint16_t seq) {
    return (seq + 1) % (NET::SEQUENCE_MAX_NUMBER_SIZE + 1);
}

void IntegrityMiddleware::processAcknowledgement(std::shared_ptr<Packet> packet) {
    JsonDocument doc;
    deserializeJson(doc, packet->getPayload().get());

    bool retry = doc["retry"].as<bool>();
    uint16_t sequence = doc["seq_num"].as<uint16_t>();
    if (retry) {
        this->resendPacketForResend(sequence);
    } else {
        this->deletePacketForResend(sequence);
    }
}

std::shared_ptr<Packet> IntegrityMiddleware::processOutgoingData(std::shared_ptr<Packet> packet) {
    packet->setNodeIdentity(MC_NAME);
    
    //we do not want to track ACK or HEARTBEAT packets
    if (packet->getMethod() == NET::HEADER::METHOD_ACKNOWLEDGEMENT || packet->getMethod() == NET::HEADER::METHOD_HEARTBEAT) return packet;
    
    packet->setSequence(this->outgoingSequence);


    if (this->ackSendEnabled) {
        //we save the packet to resend it if we get a ACK packet with the corresponding sequenceNumber and retry true - this case means that we need to resend the packet with the corresponding sequence number inlcuded in the ACK packet payload. if we get retry false, we can savely delete the entry in packetsForResend map.
        //check if there is already an entry with the sequence to avoid memory leaks.
        auto sequence = packet->getSequence();
        auto it = this->packetsForResend.find(sequence);
        if (it != this->packetsForResend.end()) {
            this->packetsForResend.erase(it);
        }

        this->packetsForResend[packet->getSequence()] = packet;

        this->outgoingSequence = this->incrementSequence(this->outgoingSequence);
    }


    return packet;
}

void IntegrityMiddleware::enableAckPackets() {
    this->ackSendEnabled = true;
}

void IntegrityMiddleware::disableAckPackets() {
    this->ackSendEnabled = false;
}

/**
 * @brief This functions sends a ACK packet to the client indicating that a packet was received or not.
 * 
 * The ACK packet has two functions: 1) confirm that a packet was received 2) indicate that a packet was not received. The second case maybe if a packet was lost on the way or when a sequence was received that indicates that there is a missing packet.
 * @param sequenceNumber of the packet sent or missing
 * @param retry true indicates the packet was received, false not
 */
void IntegrityMiddleware::sendAckPacket(uint16_t sequenceNumber, bool retry) {
    JsonDocument doc;
    doc["seq_num"] = sequenceNumber;
    doc["retry"] = retry;

    /** 
     * @brief Computes the length of the minified JSON document that serializeJson() produces, excluding the null-terminator.
     * @cite https://arduinojson.org/v7/api/json/measurejson/
     * */
    size_t bufferSize = measureJson(doc) + 1;
    char buffer[bufferSize]; // do inplace; no pointer needed unless we are moving it between functions
    serializeJson(doc, buffer, bufferSize);
    
    this->relay->ack(buffer);
}

/**
 * @brief Deletes the entries in packetsForResend map.
 * @param sequenceNumber is the sequenceNumber that is transfered in the body of a ACK packet and is also a key in packetsForResend.
 */
void IntegrityMiddleware::deletePacketForResend(uint16_t sequenceNumber) {
    auto it = this->packetsForResend.find(sequenceNumber);
    if (it != this->packetsForResend.end()) {
        this->packetsForResend.erase(it);
    }
}

/**
 * @brief If an ACK packet indicates that a packet must be resent, the sequenceNumber queries the corresponding packet so that it can directly be resent.
 * @param sequenceNumber is the sequenceNumber that is transfered in the body of a ACK packet and is also a key in packetsForResend.
 */
void IntegrityMiddleware::resendPacketForResend(uint16_t sequenceNumber) {
    auto it = this->packetsForResend.find(sequenceNumber);
    if (it != this->packetsForResend.end()) {
        this->sendAckPacket(sequenceNumber, true);
    }    
}
