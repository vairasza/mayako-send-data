#include <M5Stack.h>
#undef min
#include <vector>

#include "SensorBase.h"
#include "ButtonSensor.h"
#include "ButtonModel.h"
#include "Config.h"
#include "Logger.h"

ButtonSensor::ButtonSensor(const String &identity, String buttonName): SensorBase(identity) {
    if (buttonName == BUTT::ON_A) {
        button = &(M5.BtnA);
    } else if (buttonName == BUTT::ON_B) {
        button = &(M5.BtnB);
    } else if (buttonName == BUTT::ON_C) {
        button = &(M5.BtnC);
    } else {
        this->logger->ferror(prefix("button name %% does not exist"), std::vector<String>{ buttonName });
    }
}

String ButtonSensor::readData() {
    ButtonModel model;
    if (button == nullptr) this->logger->error(prefix("button is nullptr"));
    model.isPressed = button->isPressed();
    
    this->appendMetaData(model);
    return this->toJSON(model);
}

void ButtonSensor::identificationAction() {
    //identify button
}

void ButtonSensor::getModelDefinition(JsonObject& json) {
    ButtonModel model = ButtonModel();
    model.getModelDefinition(json);
}