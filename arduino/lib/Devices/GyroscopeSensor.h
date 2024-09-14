#ifndef GYROSCOPE_SENSOR_H
#define GYROSCOPE_SENSOR_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "SensorBase.h"

class GyroscopeSensor : public SensorBase {
    public:
        GyroscopeSensor(const String &identity);

        String readData() override;
        void identificationAction() override;
        void getModelDefinition(JsonObject& json) override;
};

#endif
