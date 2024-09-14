#include <M5Stack.h>
#undef min
#include <ArduinoJson.h>

#include "SensorBase.h"
#include "AccelerometerModel.h"
#include "AccelerometerSensor.h"
#include "ModelBase.h"

AccelerometerSensor::AccelerometerSensor(const String &identity): SensorBase(identity) {}

String AccelerometerSensor::readData() {
    AccelerometerModel model = AccelerometerModel();
    M5.Imu.getAccelData(&model.x, &model.y, &model.z);
    this->appendMetaData(model);
    
    String output = this->toJSON(model);
    
    this->lastState = output;

    return output;
}

void AccelerometerSensor::identificationAction() {
    //identifiy the accelerometer sensor
}

void AccelerometerSensor::getModelDefinition(JsonObject& json) {
    AccelerometerModel model = AccelerometerModel();
    model.getModelDefinition(json);
}