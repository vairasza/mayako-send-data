#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <Arduino.h>
#include <map>
#include <vector>
#include <memory>

#include "SensorBase.h"
#include "ActuatorBase.h"
#include "Logger.h"
#include "BoardBase.h"
#include "ModelBase.h"
#include "Capabilities.h"
#include "Identification.h"
#include "PacketRelay.h"
#include "ActuatorCommand.h"

class DeviceManager: public IDeviceCapabilities, public IIdentification {
    public:
        DeviceManager(BoardBase *board);
        ~DeviceManager();
        
        void addSensor(SensorBase *sensor);
        void addActuator(ActuatorBase *actuator);
        ActuatorBase* getActuator(const String& id);
        
        void updateSensors(); //update the board for sensors
        String getAllocatedHeap();
        bool isRecordInProgress();
        void isRecordComplete();
        std::vector<std::shared_ptr<Packet>> readSensors();
        
        //command methods
        void restart();
        void getBattery();
        void identify(JsonDocument *json);
        void readCapabilities();
        void createCapabilities(JsonDocument *json);
        void startRecord();
        void stopRecord();

    private:
        const String& identity;
        DeviceCapabilities capabilities;
        bool isRecording;
        unsigned long startTime;
        unsigned long sampleCount;
        std::map<String, SensorBase*> sensors;
        std::map<String, ActuatorBase*> actuators;
        Logger *logger;
        PacketRelay *relay;
        BoardBase *board;
        DeviceCapabilities deviceCapabilities;

        void resetCapabilities() override;
        void identificationAction() override;
        void softResetRecordCapabilities();
        void sendJsonDocument(JsonDocument& doc);
};

#endif