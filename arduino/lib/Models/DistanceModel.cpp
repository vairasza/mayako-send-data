#include <ArduinoJson.h>

#include "DistanceModel.h"

void DistanceModel::getModelDefinition(JsonObject& json) {
    json["class"] = "Distance";
    json["range"] = "int";
}

void DistanceModel::appendModelData(JsonDocument& obj) {
    obj["range"] = this->range;
}
