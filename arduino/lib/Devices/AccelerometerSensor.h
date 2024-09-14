#ifndef ACCELEROMETER_SENSOR_H
#define ACCELEROMETER_SENSOR_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "SensorBase.h"

class AccelerometerSensor : public SensorBase {
    public:
        AccelerometerSensor(const String &identity);

        String readData() override;
        void identificationAction() override;
        void getModelDefinition(JsonObject& json) override;
};

#endif
