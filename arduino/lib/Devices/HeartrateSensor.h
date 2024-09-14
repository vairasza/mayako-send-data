#ifndef HEARTRATE_SENSOR_H
#define HEARTRATE_SENSOR_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "SensorBase.h"

class HeartrateSensor : public SensorBase {
    public:
        HeartrateSensor(const String &identity);

        String readData() override;
        void identificationAction() override;
        void getModelDefinition(JsonObject& json) override;
};

#endif
