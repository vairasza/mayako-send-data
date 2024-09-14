#include <ArduinoJson.h>

#include "AccelerometerModel.h"

void AccelerometerModel::getModelDefinition(JsonObject& json) {
    json["class"] = "Accelerometer";
    json["x"] = "float";
    json["y"] = "float";
    json["z"] = "float";
}

void AccelerometerModel::appendModelData(JsonDocument &obj) {
    obj["x"] = this->x;
    obj["y"] = this->y;
    obj["z"] = this->z;
}