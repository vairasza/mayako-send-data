#ifndef SENSOR_BASE_H
#define SENSOR_BASE_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "DeviceBase.h"
#include "ModelBase.h"
#include "Capabilities.h"

class SensorBase: public DeviceBase, public ISensorCapabilities {

    public:
        SensorBase(const String& identity);
        ~SensorBase();

        SensorCapabilities* getSensorCapabilties() override;
        void setSensorCapabilities(SensorCapabilities* capabilities) override;
        void resetSensorCapabilities() override;

        virtual String readData() = 0;
        virtual void getModelDefinition(JsonObject& json) = 0;

        bool hasStateChanged(String &currentState);
        bool isTimeToRun();
        unsigned long getSequence();
        void resetSequence();

        void enable();
        void disable();
        bool isEnabled();

    protected:
        unsigned long interval;
        String lastState;
        SensorCapabilities capabilities;
        unsigned long lastRun;
        unsigned long sequence;
        
        void appendMetaData(ModelBase &model);
        String toJSON(ModelBase &model);
        void calculateInterval();
};

#endif