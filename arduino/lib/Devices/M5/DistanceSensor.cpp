#include <M5Stack.h>
#undef min
#include <VL53L0X.h>

#include "DistanceSensor.h"
#include "DistanceModel.h"

DistanceSensor::DistanceSensor(const String &identity): SensorBase(identity) {
    this->sensor.setTimeout(500);
    if (!this->sensor.init()) {
        this->logger->error(prefix("failed to detect or init distance sensor"));
    }
    this->sensor.startContinuous();
}

String DistanceSensor::readData() {
    DistanceModel model = DistanceModel();
    int range = sensor.readRangeSingleMillimeters();
    model.range = range;
    this->appendMetaData(model);
    return this->toJSON(model);
}

void DistanceSensor::identificationAction() {
    
}

void DistanceSensor::getModelDefinition(JsonObject& json) {
    DistanceModel model = DistanceModel();
    model.getModelDefinition(json);
}
