#ifndef DISTANCE_SENSOR_H
#define DISTANCE_SENSOR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <VL53L0X.h>

#include "SensorBase.h"

class DistanceSensor : public SensorBase {
    public:
        DistanceSensor(const String &identity);

        String readData() override;
        void identificationAction() override;
        void getModelDefinition(JsonObject& json) override;

    private:
        VL53L0X sensor;
};

#endif
