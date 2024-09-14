#include <M5Stack.h>
#undef min

#include "TemperatureSensor.h"
#include "SensorBase.h"
#include "TemperatureModel.h"

TemperatureSensor::TemperatureSensor(const String &identity): SensorBase(identity) {}

String TemperatureSensor::readData()
{
    TemperatureModel model = TemperatureModel();
    M5.Imu.getTempData(&model.temperature);
    
    this->appendMetaData(model);
    return this->toJSON(model);
}

void TemperatureSensor::identificationAction() {
    //identify temperature sensor
}

void TemperatureSensor::getModelDefinition(JsonObject& json) {
    TemperatureModel model = TemperatureModel();
    model.getModelDefinition(json);
}