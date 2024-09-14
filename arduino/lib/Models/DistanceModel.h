#ifndef DISTANCE_MODEL_H
#define DISTANCE_MODEL_H

#include <Arduino.h>

#include "ModelBase.h"

class DistanceModel: public ModelBase {
    public:
        DistanceModel() : ModelBase(), range(0) {}

        int range;
    
        void getModelDefinition(JsonObject& json) override;
        void appendModelData(JsonDocument& obj) override;
};

#endif