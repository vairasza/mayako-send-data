#ifndef TEMPERATURE_MODEL_H
#define TEMPERATURE_MODEL_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "ModelBase.h"

class TemperatureModel: public ModelBase {
    public:
        TemperatureModel() : ModelBase(), temperature(0.0f) {}

        float temperature;
    
        void getModelDefinition(JsonObject& json) override;
        void appendModelData(JsonDocument& obj) override;
};

#endif