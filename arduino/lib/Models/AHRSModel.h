#ifndef AHRS_MODEL_H
#define AHRS_MODEL_H

#include <Arduino.h>

#include "ModelBase.h"

class AHRSModel: public ModelBase {
    public:
        AHRSModel() : ModelBase(), pitch(0.0f), roll(0.0f), yaw(0.0f) {}

        float pitch;
        float roll;
        float yaw;
        
        void getModelDefinition(JsonObject& json) override;
        void appendModelData(JsonDocument& obj) override;
};

#endif