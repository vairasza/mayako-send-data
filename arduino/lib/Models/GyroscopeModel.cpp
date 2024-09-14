#include <ArduinoJson.h>

#include "GyroscopeModel.h"

void GyroscopeModel::getModelDefinition(JsonObject& json) {
    json["class"] = "Gyroscope";
    json["x"] = "float";
    json["y"] = "float";
    json["z"] = "float";
}

void GyroscopeModel::appendModelData(JsonDocument &obj)
{
    obj["x"] = this->x;
    obj["y"] = this->y;
    obj["z"] = this->z;
}