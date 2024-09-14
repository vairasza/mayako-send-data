#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <memory>

#include "DeviceManager.h"
#include "SensorBase.h"
#include "ActuatorBase.h"
#include "ModelBase.h"
#include "Config.h"
#include "Packet.h"
#include "PacketRelay.h"
#include "Definitions.h"

DeviceManager::DeviceManager(BoardBase *board) : identity(MC_NAME), board(board), startTime(0), isRecording(false)
{
    this->deviceCapabilities = DeviceCapabilities();
    this->resetCapabilities();

    this->logger = Logger::getInstance();
    this->relay = PacketRelay::getInstance();

    this->board->init();
}

DeviceManager::~DeviceManager()
{
    this->sensors.clear();
    this->actuators.clear();
    this->logger = nullptr;
}

void DeviceManager::addSensor(SensorBase *sensor)
{
    this->sensors[sensor->identity] = sensor;
}

void DeviceManager::addActuator(ActuatorBase *actuator)
{
    this->actuators[actuator->identity] = actuator;
}

ActuatorBase *DeviceManager::getActuator(const String &id)
{
    if (this->actuators.find(id) != this->actuators.end())
    {
        return this->actuators[id];
    }

    return nullptr;
}

void DeviceManager::updateSensors()
{
    this->board->update();
}

String DeviceManager::getAllocatedHeap()
{
    return String(this->board->getAllocatedHeap());
}

/**
 * @brief restarts the device and loads default parameters; device needs to reconnect BLE if applied
 * @note used in commandmanager
 */
void DeviceManager::restart()
{
    // logging not included because it would be sent after restart was executed
    this->board->restart();
}

/**
 * @brief The start command triggers the record, setting isRecording to true. In the main loop we call this function and need to wait until the delay is over.
 */
bool DeviceManager::isRecordInProgress()
{
    unsigned long now = millis();
    
    // this if statement provides a delay after the start command was triggered
    if (now < this->startTime + this->deviceCapabilities.delay)
    {

        return false;
    }

    return this->isRecording;
}

/**
 * @brief the record can not only be stopped with the stop command, device capabilities provides the duration and maxSamples which can also end the record
 */
void DeviceManager::isRecordComplete()
{
    unsigned long now = millis();

    // duration only triggers if set to above 0
    if (this->capabilities.duration > 0 && now > this->startTime + this->capabilities.duration)
    {
        this->stopRecord();
    }

    // max samples only triggers if set to above 0
    if (this->capabilities.maxSamples > 0 && this->sampleCount >= this->capabilities.maxSamples)
    {
        this->stopRecord();
    }
}

std::vector<std::shared_ptr<Packet>> DeviceManager::readSensors()
{
    std::vector<std::shared_ptr<Packet>> output;

    for (auto it : this->sensors)
    {
        auto sensor = it.second;
        
        if (!sensor->isEnabled())
            continue;        
        if (!sensor->isTimeToRun())
            continue;        

        String data = sensor->readData();

        if (!sensor->hasStateChanged(data))
            continue;        

        this->sampleCount++; // TODO: for all samples?    
        std::shared_ptr<Packet> packet = std::make_shared<Packet>();
        packet->setMethod(NET::HEADER::METHOD_DATA);
        packet->setPayload(data.c_str());

        output.push_back(std::move(packet));
    }

    return output;
}

/**
 * @brief identifies the microcontroller by sending a package which includes the feedback name and the identity of the microcontroller (this is already included in the packet header, therefore we dont include it in the payload)
 * @note used in commandmanager
 */
void DeviceManager::identify(JsonDocument *json)
{
    String id = (*json)["identity"].as<String>();

    // check if microcontroller name matches
    if (id == this->identity)
        this->identificationAction();

    // check if sensor identity matches
    auto iteratorSensor = this->sensors.find(id);
    if (iteratorSensor != this->sensors.end())
        iteratorSensor->second->identificationAction();

    // check if actuator identity matches
    auto iteratorActuator = this->actuators.find(id);
    if (iteratorActuator != this->actuators.end())
        iteratorActuator->second->identificationAction();
}

/**
 * @brief gives information about the condition of the battery - % and charging
 * @note used in commandmanager
 */
void DeviceManager::getBattery()
{
    int battery = this->board->getBattery();
    bool isCharging = this->board->getBatteryCharging();

    JsonDocument doc;

    doc["name"] = CMD::BATTERY_READ;
    doc["percentage"] = String(battery);
    doc["charging"] = String(isCharging);

    this->sendJsonDocument(doc);
}

/**
 * @brief get information about the capabilities from the microcontroller and all sensors and actuators
 * @note used in commandmanager
 */
void DeviceManager::readCapabilities()
{
    JsonDocument doc;
    doc["name"] = CMD::RECORD_READ;
    doc["duration"] = this->capabilities.duration;
    doc["max_samples"] = this->capabilities.maxSamples;
    doc["delay"] = this->capabilities.delay;

    JsonArray docSensors = doc["sensors"].to<JsonArray>();
    for (auto &sensor : this->sensors)
    {
        SensorCapabilities *sc = sensor.second->getSensorCapabilties();

        JsonObject s = docSensors.add<JsonObject>();
        s["identity"] = sensor.second->identity;
        s["enable"] = sc->enable;
        s["include_timestamp"] = sc->includeTimestamp;
        s["include_sequence"] = sc->includeSequence;
        s["sample_rate"] = sc->sampleRate;
        s["data_on_state_change"] = sc->dataOnStateChange;

        JsonObject jObject = s["model_data"].to<JsonObject>();
        sensor.second->getModelDefinition(jObject);
    }

    JsonArray docActuators = doc["actuators"].to<JsonArray>();
    for (auto actuator : this->actuators)
    {
        ActuatorCapabilities *ac = actuator.second->getActuatorCapabilties();

        JsonObject s = docActuators.add<JsonObject>();
        s["identity"] = actuator.second->identity;
        s["enable"] = ac->enable;
        JsonArray jArray = s["commands"].to<JsonArray>();
        actuator.second->getCommandsDefinition(jArray);
    }

    this->sendJsonDocument(doc);
}

/**
 * @note used in commandmanager
 */
void DeviceManager::createCapabilities(JsonDocument *json)
{
    JsonDocument doc;
    doc["name"] = CMD::RECORD_CREATE;

    if (this->isRecording)
    {
        doc["success"] = false;
        doc["error"] = "can not create new record because there is currently a running record";
    }
    else
    {
        auto data = *json;

        this->deviceCapabilities.delay = data["delay"].as<int>();
        this->deviceCapabilities.duration = data["duration"].as<unsigned long>();
        this->deviceCapabilities.maxSamples = data["max_samples"].as<unsigned long>();
        bool includeTimestamp = data["include_timestamp"].as<bool>();
        bool includeSequence = data["include_sequence"].as<bool>();

        JsonArray dataSensors = data["sensors"];
        for (JsonVariant item : dataSensors)
        {
            String idx = item["identity"];
            auto it = this->sensors.find(idx);
            if (it == this->sensors.end())
                continue;

            SensorCapabilities senCap = SensorCapabilities();
            senCap.enable = item["enable"].as<bool>();
            senCap.includeTimestamp = includeTimestamp;
            senCap.includeSequence = includeSequence;
            senCap.sampleRate = item["sample_rate"].as<unsigned long>();
            senCap.dataOnStateChange = item["data_on_state_change"].as<bool>();

            it->second->setSensorCapabilities(&senCap);
        }

        JsonArray dataActuators = data["actuators"];
        for (JsonVariant item : dataActuators)
        {
            String idx = item["identity"];
            auto it = this->actuators.find(idx);
            if (it == this->actuators.end())
                continue;

            ActuatorCapabilities actCap = ActuatorCapabilities();
            actCap.enable = item["enable"].as<bool>();

            it->second->setActuatorCapabilities(&actCap);
        }

        doc["success"] = true;
    }

    this->sendJsonDocument(doc);
}

/**
 * @brief starts a record that is delayed by a set amount if it is not running
 * @note used in commandmanager
 */
void DeviceManager::startRecord()
{
    JsonDocument doc;
    doc["name"] = CMD::RECORD_START;
    

    if (this->isRecording)
    {
        // if it is already recording, we do not want to stop or restart it. so we make a failed respons back to the user.
        doc["status"] = String(this->isRecording);
        doc["success"] = false;
        doc["error"] = "can not start record because it is already running";
    }
    else
    {
        // here we set the startTime, isRecording to true and in the loop we constantly call hasStarted which checks if the delay has passed
        this->sampleCount = 0;
        this->startTime = millis();
        this->isRecording = true;

        doc["status"] = String(this->isRecording);
        doc["success"] = true;
    }

    this->sendJsonDocument(doc);
}

/**
 * @brief stops a record if it is running
 * @note used in commandmanager
 */
void DeviceManager::stopRecord()
{
    JsonDocument doc;
    doc["name"] = CMD::RECORD_STOP;
    doc["status"] = String(this->isRecording);

    if (!this->isRecording)
    {
        // their is currently no running record. so we can not stop it.
        doc["success"] = false;
        doc["error"] = "can not stop record because it is not running";
    }
    else
    {
        // here we stop the record successfully and make a soft reset on the capabilities.
        this->isRecording = false;
        this->softResetRecordCapabilities();

        doc["success"] = true;
    }

    this->sendJsonDocument(doc);
}

void DeviceManager::resetCapabilities()
{
    this->capabilities.duration = DEVICE::DURATION;
    this->capabilities.maxSamples = DEVICE::MAX_SAMPLES;
    this->capabilities.delay = DEVICE::DELAY;
}

void DeviceManager::softResetRecordCapabilities()
{
    for (auto &sensor : this->sensors)
    {
        sensor.second->resetSequence();
    }
}

void DeviceManager::identificationAction()
{
    this->board->identify();
}

void DeviceManager::sendJsonDocument(JsonDocument &doc)
{
    size_t bufferSize = measureJson(doc) + 1;
    std::unique_ptr<char[]> buffer(new char[bufferSize]);
    serializeJson(doc, buffer.get(), bufferSize);
    this->relay->info(buffer.get());
    this->logger->debug(prefix("sending capa"));
}