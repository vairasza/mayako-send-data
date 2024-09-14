#include <M5Stack.h>
#undef min

#include "GyroscopeSensor.h"
#include "GyroscopeModel.h"

GyroscopeSensor::GyroscopeSensor(const String &identity): SensorBase(identity) {}

String GyroscopeSensor::readData()
{
    GyroscopeModel model = GyroscopeModel();
    M5.Imu.getGyroData(&model.x, &model.y, &model.z);
    
    this->appendMetaData(model);
    return this->toJSON(model);
}

void GyroscopeSensor::identificationAction() {
    //identify gyroscope
}

void GyroscopeSensor::getModelDefinition(JsonObject& json) {
    GyroscopeModel model = GyroscopeModel();
    model.getModelDefinition(json);
}