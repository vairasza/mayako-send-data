#include <M5Stack.h>
#undef min
#include "MAX30100_PulseOximeter.h"

#include "HeartrateSensor.h"
#include "HeartrateModel.h"

PulseOximeter pox;

HeartrateSensor::HeartrateSensor(const String &identity): SensorBase(identity) {
    if (!pox.begin()) {
        this->logger->error(prefix("heartrate sensor failed to initialise"));
    }
}

String HeartrateSensor::readData() {
    pox.update();
    HeartrateModel model = HeartrateModel();
    model.heartRate = pox.getHeartRate();
    model.sp02 = pox.getSpO2();
    this->appendMetaData(model);
    return this->toJSON(model);
}

void HeartrateSensor::identificationAction() {
    //identify heartrate sensor
}

void HeartrateSensor::getModelDefinition(JsonObject& json) {
    HeartrateModel model = HeartrateModel();
    model.getModelDefinition(json);
}