#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "SensorBase.h"

class TemperatureSensor: public SensorBase {
    public:
        TemperatureSensor(const String& identity);
    
        String readData() override;
        void identificationAction() override;
        void getModelDefinition(JsonObject& json) override;
};

#endif
