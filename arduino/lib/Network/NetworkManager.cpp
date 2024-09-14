#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <queue>
#include <memory>

#include "NetworkManager.h"
#include "SerialProtocol.h"
#include "Config.h"
#include "Storage.h"
#include "IntegrityMiddleware.h"
#include "Packet.h"
#include "Logger.h"
#include "PacketRelay.h"
#include "Definitions.h"

#if WIRELESS_MODE == BLE
#include "BluetoothProtocol.h"
#elif WIRELESS_MODE == WIFI
#include "WifiProtocol.h"
#endif

NetworkManager::NetworkManager(Storage *storage) : currentProtocol(nullptr), lastHeartBeat(0), upgradeProtocolTimeout(0)
{
    // init of logging and relay classes; set networkqueue so that we can process it here
    this->logger = Logger::getInstance();
    this->logger->setQueue(&(this->output));
    this->relay = PacketRelay::getInstance();
    this->relay->setQueue(&(this->output));

    // network implementations init
    this->serialProtocol = new SerialProtocol();
    this->serialProtocol->init();
    this->currentProtocol = this->serialProtocol;

#if WIRELESS_MODE == BLE
    this->bluetoothProtocol = new BluetoothProtocol();
    this->bluetoothProtocol->init();
#elif WIRELESS_MODE == WIFI
    this->wifiProtocol = new WifiProtocol(storage);
    this->wifiProtocol->init();
#endif

    // integrity manager
    this->integrityMiddleware = IntegrityMiddleware();
}

NetworkManager::~NetworkManager() {}

std::vector<JsonDocument> NetworkManager::readIncomingData()
{    
    std::vector<JsonDocument> output;    
    std::shared_ptr<Packet> packet = this->currentProtocol->readPacket();    
    if (packet == nullptr)
        return output;    

    std::vector<std::shared_ptr<Packet>> result = this->integrityMiddleware.processIncomingData(std::move(packet));    
    for (std::shared_ptr<Packet> data : result)
    {
        JsonDocument doc;

        DeserializationError error = deserializeJson(doc, data->getPayload().get());        
        switch (data->getMethod())
        {
        case NET::HEADER::METHOD_COMMAND:            
            output.push_back(doc);

            break;
        case NET::HEADER::METHOD_HEARTBEAT:
            // this->heartbeatMonitor.registerHeartbeat();
            break;
        default:
            break;
        }

        if (error)
        {

            this->logger->ferror(prefix("can not deserialise json string: %%"), std::vector<String>{error.c_str()});

            return output;
        }
    }    
    return output;
}

void NetworkManager::writeOutgoingData()
{
    while (!this->output.empty())
    {
        std::shared_ptr<Packet> nextPacket = this->output.front();
        this->output.pop();

        std::shared_ptr<Packet> notedPacket = this->integrityMiddleware.processOutgoingData(std::move(nextPacket));

        if (notedPacket != nullptr)
        {
            this->currentProtocol->writePacket(std::move(notedPacket));
        }
    }
}

void NetworkManager::addSensorDataToOutput(std::vector<std::shared_ptr<Packet>> &sensorData)
{
    for (std::shared_ptr<Packet> item : sensorData)
    {
        this->output.push(item);
    }
}

bool NetworkManager::isConnected()
{
    return this->currentProtocol->checkConnection();
}

/**
 * @brief frequent check that upgrades to wireless mode if there is a connection
 * 
 * first, the function introduces a timeout that is defined at NET::TIMEOUT_WIRELESS_UPGRADE. this is to avoid too frequent change of protocols. then guards are included to choose between BLE and WiFi depending on the build on this device. the protocol is upgraded to wireless if the protocol is not already wireless and if the wireless protocol has an active connection. then switch to use this protocol. if the connection drops (this excludes the first if statement), and the current protocol is not already serial (this avoid unnessary reassining), we set it to serial. this ensures a simple upgrade to wireless with serial as fallback.
 */
void NetworkManager::upgradeProtocol()
{
    if (this->checkTimeout(this->upgradeProtocolTimeout, NET::TIMEOUT_WIRELESS_UPGRADE))
    {
        #if WIRELESS_MODE == BLE
        if (this->currentProtocol != this->bluetoothProtocol && this->bluetoothProtocol->checkConnection())
        {
            this->currentProtocol = this->bluetoothProtocol;
        }
        #elif WIRELESS_MODE == WIFI
        if (this->currentProtocol != this->wifiProtocol && this->wifiProtocol->checkConnection())
        {
            this->currentProtocol = this->wifiProtocol;
        }
        #endif
        //if currentprotocol is bluetooth and not connected, set to serial
        else if (this->currentProtocol != this->serialProtocol)
        {
            this->currentProtocol = this->serialProtocol;
        }
        //if currentprotocol is already serial, we dont ned to reset it
    }
}

void NetworkManager::sendHeartbeatToClient()
{
    if (this->checkTimeout(this->lastHeartBeat, NET::HEARTBEAT_INTERVAL))
        this->relay->heartbeat();
}

bool NetworkManager::checkTimeout(unsigned long &lastTimeout, unsigned long interval)
{
    unsigned long now = millis();

    if (now - lastTimeout >= interval)
    {
        lastTimeout = now;
        return true;
    }

    return false;
}

void NetworkManager::readConnection()
{
    JsonDocument doc;
    doc["name"] = CMD::CONNECTION_READ;
    doc["protocol"] = this->currentProtocol->getName();
    doc["connection"] = this->currentProtocol->checkConnection();

    this->sendJsonDocument(doc);
}

#if WIRELESS_MODE == BLE
#elif WIRELESS_MODE == WIFI
void NetworkManager::readActiveWifiProfile(JsonDocument *json)
{
    JsonDocument doc;
    doc["name"] = CMD::WIFI_PROFILE_ACTIVE_READ;

    bool success = this->wifiProtocol->readActiveProfile(doc);

    doc["success"] = success;
    if (!success)
    {
        doc["error"] = "could not create the active WiFi profile";
    }

    this->sendJsonDocument(doc);
}

/**
 * @brief creates a new wifi profile in the internal storage of the microcontroller. The json parameter is translated to JsonDocument in NetworkManager::readPacket.
 * @param JsonDocument
 */
void NetworkManager::createWifiProfile(JsonDocument *json)
{
    String key = (*json)["wifi_key"].as<String>();

    JsonDocument doc;
    doc["name"] = CMD::WIFI_PROFILE_CREATE;
    doc["wifi_key"] = key;

    if (key.isEmpty())
    {
        doc["success"] = false;
        doc["error"] = "could not read wifi_key from request body";
    }
    else
    {
        size_t profileSize = measureJson(*json) + 1;
        char profile[profileSize];

        serializeJson(*json, profile, profileSize);

        bool success = this->wifiProtocol->createProfile(key, profile);
        doc["success"] = success;

        if (!success)
        {
            doc["error"] = "could not create a profile with wifi_key";
        }
    }

    this->sendJsonDocument(doc);
}

void NetworkManager::readWifiProfile(JsonDocument *json)
{
    String key = (*json)["wifi_key"].as<String>();

    JsonDocument doc;
    doc["name"] = CMD::WIFI_PROFILE_READ;

    if (key.isEmpty())
    {
        doc["success"] = false;
        doc["error"] = "could not read wifi_key from request body";
    }
    else
    {
        bool success = this->wifiProtocol->readProfile(key, doc);
        doc["success"] = success;

        if (!success)
        {
            doc["error"] = "could not read a profile with wifi_key";
        }
    }

    this->sendJsonDocument(doc);
}

void NetworkManager::readWifiAllProfiles(JsonDocument *json)
{
    JsonDocument doc;
    doc["name"] = CMD::WIFI_PROFILE_ALL_READ;
    JsonArray jArray = doc["profiles"].to<JsonArray>();

    this->wifiProtocol->readAllProfiles(jArray);

    this->sendJsonDocument(doc);
}

void NetworkManager::selectActiveWifiProfile(JsonDocument *json)
{
    String key = (*json)["wifi_key"].as<String>();

    JsonDocument doc;
    doc["name"] = CMD::WIFI_PROFILE_ACTIVE_SELECT;
    doc["wifi_key"] = key;

    if (key.isEmpty())
    {
        doc["success"] = false;
        doc["error"] = "could not read wifi_key from request body";
    }
    else
    {
        bool success = this->wifiProtocol->selectActiveProfile(key);

        doc["success"] = success;
        if (!success)
        {
            doc["error"] = "wifi key does not exist or wifi profile could not be selected";
        }
    }

    this->sendJsonDocument(doc);
}

void NetworkManager::destroyWifiProfile(JsonDocument *json)
{
    String key = (*json)["wifi_key"].as<String>();

    JsonDocument doc;
    doc["name"] = CMD::WIFI_PROFILE_DELETE;
    doc["wifi_key"] = key;

    if (key.isEmpty())
    {
        doc["success"] = false;
        doc["error"] = "could not read wifi_key from request body";
    }
    else
    {
        bool success = this->wifiProtocol->destroyProfile(key);

        doc["success"] = success;
        if (!success)
        {
            doc["error"] = "wifi key does not exist or wifi profile could not be destroyed";
        }
    }

    this->sendJsonDocument(doc);
}
#endif

void NetworkManager::enableAckPackets()
{
    this->integrityMiddleware.enableAckPackets();

    JsonDocument doc;
    doc["name"] = CMD::ACKNOWLEDGEMENT_ENABLE;
    doc["status"] = true;
    doc["success"] = true;

    this->sendJsonDocument(doc);
}

void NetworkManager::disableAckPackets()
{
    this->integrityMiddleware.disableAckPackets();

    JsonDocument doc;
    doc["name"] = CMD::ACKNOWLEDGEMENT_DISABLE;
    doc["status"] = false;
    doc["success"] = true;

    this->sendJsonDocument(doc);
}

void NetworkManager::sendJsonDocument(JsonDocument &doc)
{
    size_t bufferSize = measureJson(doc) + 1;
    std::unique_ptr<char[]> buffer(new char[bufferSize]);
    serializeJson(doc, buffer.get(), bufferSize);
    this->relay->info(buffer.get());
}