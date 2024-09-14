#ifndef ACCELEROMETER_MODEL_H
#define ACCELEROMETER_MODEL_H

#include <Arduino.h>

#include "ModelBase.h"

class AccelerometerModel: public ModelBase {
    public:
        AccelerometerModel() : ModelBase(), x(0.0f), y(0.0f), z(0.0f) {}
        
        float x;
        float y;
        float z;

        void getModelDefinition(JsonObject& json) override;
        void appendModelData(JsonDocument& obj) override;
};

#endif