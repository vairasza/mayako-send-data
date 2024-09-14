#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <memory>
#include <math.h>

#include "BluetoothProtocol.h"
#include "Config.h"
#include "Logger.h"
#include "Definitions.h"

BluetoothProtocol::BluetoothProtocol() : ProtocolBase(NET::BLE_NAME, NET::MAX_BUFFER_SIZE), BLECharacteristicCallbacks(), BLEServerCallbacks(), server(nullptr), service(nullptr), characteristic(nullptr), maxPayloadSize(NET::BLE_EXPECTED_MTU)
{
    this->logger = Logger::getInstance();
}

BluetoothProtocol::~BluetoothProtocol()
{
    this->cleanup();
    this->logger->debug(prefix("BLE destruct"));
}

void BluetoothProtocol::init()
{
    BLEDevice::init(MC_NAME);

    this->server = BLEDevice::createServer();
    this->server->setCallbacks(this);

    this->service = this->server->createService(SERVICE_UUID);

    this->characteristic = this->service->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE);
    this->characteristic->setCallbacks(this);
    this->characteristic->addDescriptor(new BLE2902());

    this->service->start();
    BLEAdvertising *adv = this->server->getAdvertising();
    adv->addServiceUUID(SERVICE_UUID);
    adv->start();

    this->logger->debug(prefix("BLE initiated"));
}

void BluetoothProtocol::destroy()
{
    this->logger->debug(prefix("BLE destroy"));
    if (this->checkConnection())
    {
        // here we can just us 0 as client id because we do not let more clients connect; also advertising stops when a client connects, therefore there can not be more than 1 client at the same time
        this->server->disconnect(0);
    }

    this->cleanup();
}

/**
 * @note we add -1 to the dataSize because we do not want to send the packet with the terminating \x00 byte. we can not add it before because we need to consider the terminating char while handling it in the packet class.
 */
void BluetoothProtocol::writePacket(std::shared_ptr<Packet> packet)
{
    if (!this->connected)
    {
        return;
    }

    uint8_t *data = packet->serialize();
    size_t dataSize = packet->getPacketSize() - 1;

    for (int i = 0; i < dataSize; i += this->maxPayloadSize)
    {
        size_t chunkSize = min(this->maxPayloadSize, dataSize - i);

        this->characteristic->setValue(data + i, chunkSize);
        this->characteristic->notify();

        delay(NET::BLE_CHUNK_TIMEOUT);
    }

    delete[] data;
}

std::shared_ptr<Packet> BluetoothProtocol::readPacket()
{
    if (this->dataCache.empty())
        return nullptr;

    std::shared_ptr<Packet> packet = this->dataCache.front();
    this->dataCache.pop();

    return std::move(packet);
}

bool BluetoothProtocol::checkConnection()
{
    return this->connected;
}

void BluetoothProtocol::cleanup()
{
    while (!this->dataCache.empty())
    {
        this->dataCache.pop();
    }

    if (this->server)
    {
        this->server->getAdvertising()->stop();
        BLEDevice::deinit(false);
        this->server = nullptr;
        this->service = nullptr;
        this->characteristic = nullptr;
    }
}

void BluetoothProtocol::onWrite(BLECharacteristic *pCharacteristic)
{
    uint8_t *data = pCharacteristic->getData();
    size_t dataSize = pCharacteristic->getValue().length();
    size_t index = 0;
    while (index < dataSize && !Packet::verifyFlag(data[index]))
    {
        index++;
    }

    if (index >= dataSize && dataSize < NET::HEADER::SIZE)
        return;
    uint8_t header[NET::HEADER::SIZE];
    memcpy(header, data + index, NET::HEADER::SIZE);

    std::shared_ptr<Packet> packet = std::make_shared<Packet>();
    packet->deserializeHeader(header);
    uint16_t payloadSize = packet->getPayloadSize();

    if (dataSize - NET::HEADER::SIZE < payloadSize)
        return;

    char payloadBuffer[payloadSize + 1];
    memcpy(payloadBuffer, data + index + NET::HEADER::SIZE, payloadSize);
    payloadBuffer[payloadSize] = '\0';

    packet->deserializePayload(payloadBuffer);

    this->dataCache.push(std::move(packet));
}

void BluetoothProtocol::advertise()
{
    if (this->server == nullptr)
        return;

    this->server->getAdvertising()->start();
}

void BluetoothProtocol::onConnect(BLEServer *pServer)
{
    this->connected = true;

    pServer->updatePeerMTU(pServer->getConnId(), NET::BLE_EXPECTED_MTU);
    size_t mtu = pServer->getPeerMTU(pServer->getConnId());

    this->maxPayloadSize = mtu - NET::BLE_ATT_OVERHEAD;
}

void BluetoothProtocol::onDisconnect(BLEServer *pServer)
{
    this->connected = false;

    // after the client has disconnected from the BLE server, we instantly restart advertising that that a reconnection is possible.
    BLEAdvertising *adv = this->server->getAdvertising();
    adv->addServiceUUID(SERVICE_UUID);
    adv->start();
}