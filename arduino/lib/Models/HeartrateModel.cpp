#include "HeartrateModel.h"

void HeartrateModel::getModelDefinition(JsonObject& json) {
    json["class"] = "Heartrate";
    json["heartrate"] = "float";
    json["sp02"] = "uint8_t";
}

void HeartrateModel::appendModelData(JsonDocument& obj) {
    obj["heartrate"] = this->heartRate;
    obj["sp02"] = this->sp02;
}