#ifndef BUTTON_SENSOR_H
#define BUTTON_SENSOR_H

#include <M5Stack.h>
#undef min
#include <ArduinoJson.h>

#include "SensorBase.h"

class ButtonSensor : public SensorBase {
    public:
        ButtonSensor(const String &identity, String buttonName);

        String readData() override;
        void identificationAction() override;
        void getModelDefinition(JsonObject& json) override;

    private:
        Button *button;
};

#endif
