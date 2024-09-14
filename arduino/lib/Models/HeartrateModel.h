#ifndef HEARTRATE_MODEL_H
#define HEARTRATE_MODEL_H

#include <Arduino.h>

#include "ModelBase.h"

class HeartrateModel: public ModelBase {
    public:
        HeartrateModel() : ModelBase(), heartRate(0), sp02(0) {}

        float heartRate;
        uint8_t sp02;
    
        void getModelDefinition(JsonObject& json) override;
        void appendModelData(JsonDocument& obj) override;
};

#endif