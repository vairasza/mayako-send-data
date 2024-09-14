#include <M5Stack.h>
#undef min

#include "AHRSSensor.h"
#include "SensorBase.h"
#include "AHRSModel.h"

AHRSSensor::AHRSSensor(const String &identity): SensorBase(identity) {}

String AHRSSensor::readData()
{
    AHRSModel model = AHRSModel();
    M5.Imu.getAhrsData(&model.pitch, &model.roll, &model.yaw);
    
    this->appendMetaData(model);
    return this->toJSON(model);
}

void AHRSSensor::identificationAction() {
    //identify AHRS sensor
}

void AHRSSensor::getModelDefinition(JsonObject& json) {
    AHRSModel model = AHRSModel();
    model.getModelDefinition(json);
}