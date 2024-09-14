#include <ArduinoJson.h>

#include "TemperatureModel.h"

void TemperatureModel::getModelDefinition(JsonObject& json) {
    json["class"] = "Temperature";
    json["temperature"] = "float";
}

void TemperatureModel::appendModelData(JsonDocument &obj)
{
    obj["temperature"] = this->temperature;
}