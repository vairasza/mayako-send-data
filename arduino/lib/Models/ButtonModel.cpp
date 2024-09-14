#include <ArduinoJson.h>

#include "ButtonModel.h"

void ButtonModel::getModelDefinition(JsonObject& json) {
    json["class"] = "Button";
    json["is_pressed"] = "boolean";
}

void ButtonModel::appendModelData(JsonDocument &obj)
{
    obj["is_pressed"] = this->isPressed;
}
