#ifndef AHRS_SENSOR_H
#define AHRS_SENSOR_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "SensorBase.h"

class AHRSSensor : public SensorBase {
    public:
        AHRSSensor(const String& identity);
        
        String readData() override;
        void identificationAction() override;
        void getModelDefinition(JsonObject& json) override;
};

#endif
