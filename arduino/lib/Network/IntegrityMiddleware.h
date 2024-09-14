#ifndef INTEGRITY_MIDDLEWARE_H
#define INTEGRITY_MIDDLEWARE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <vector>
#include <queue>
#include <memory>

#include "ProtocolBase.h"
#include "Packet.h"
#include "Logger.h"
#include "PacketRelay.h"
#include "Definitions.h"

//this class stands between the networkmanager and the actual protocol implementations and is responsible for sending ACK packet, tracking sequence, checksum checks and building the packages
class IntegrityMiddleware {
    public:
        IntegrityMiddleware();
        ~IntegrityMiddleware();

        std::vector<std::shared_ptr<Packet>> processIncomingData(std::shared_ptr<Packet> packet);
        std::shared_ptr<Packet> processOutgoingData(std::shared_ptr<Packet> packet);
        void enableAckPackets();
        void disableAckPackets();

    private:
        Logger *logger;
        PacketRelay *relay;

    // INCOMING DATA
        /* this is the last sequence where we receveived a in-order ACK packet. */
        uint16_t expectedSequence;
        /* packets that are received but have a higher sequence than expected are saved here until the expected packet arives. if the size of the map grows beyond a maximum size which still must be evaluated, we retrieve the next nearest packet to expectedSequence and increment expectedSequence by one, expecting the next higher sequence. */
        std::map<uint16_t, std::shared_ptr<Packet>> outOfOrderPackets;

        bool compareSequenceWithOverflow(uint16_t incomingSequence, uint16_t expectedSequence);
        void processAcknowledgement(std::shared_ptr<Packet> packet);
        void processValidPacket(std::shared_ptr<Packet> packet, std::vector<std::shared_ptr<Packet>> &output);
        void processOutOfOrderPackets(std::vector<std::shared_ptr<Packet>> &output);
        void handlePacketOverflow(std::vector<std::shared_ptr<Packet>> &output);
        void requestMissingPackets(std::shared_ptr<Packet> packet);  

    // OUTGOING DATA
        /**
         * Packets for resend is a map the saves packets that are not ACK that were already sent over the network. The IntegrityManager keeps them until a corresponding ACK returns which indicates wether the packet must be resent or not. If it must not be resent, the entry can be safely deleted from the map.
         */
        std::map<uint16_t, std::shared_ptr<Packet>> packetsForResend;
        uint16_t outgoingSequence;
        bool ackSendEnabled;
    
        void sendAckPacket(uint16_t sequenceNumber, bool retry);
        void deletePacketForResend(uint16_t sequenceNumber);
        void resendPacketForResend(uint16_t sequenceNumber);
        uint16_t incrementSequence(uint16_t seq);
};

#endif