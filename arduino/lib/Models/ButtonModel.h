#ifndef BUTTON_MODEL_H
#define BUTTON_MODEL_H

#include <Arduino.h>

#include "ModelBase.h"

class ButtonModel: public ModelBase {
    public:
        ButtonModel() : ModelBase(), isPressed(false) {}

        bool isPressed;

        void getModelDefinition(JsonObject& json) override;
        void appendModelData(JsonDocument& obj) override;
};

#endif