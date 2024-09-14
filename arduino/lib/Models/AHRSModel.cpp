#include <ArduinoJson.h>

#include "AHRSModel.h"

void AHRSModel::getModelDefinition(JsonObject& json) {
    json["class"] = "AHRS";
    json["pitch"] = "float";
    json["roll"] = "float";
    json["yaw"] = "float";
}

void AHRSModel::appendModelData(JsonDocument &obj)
{
    obj["pitch"] = this->pitch;
    obj["roll"] = this->roll;
    obj["yaw"] = this->yaw;
}