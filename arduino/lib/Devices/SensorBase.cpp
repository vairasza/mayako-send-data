#include "SensorBase.h"
#include "Config.h"
#include "ModelBase.h"
#include "Capabilities.h"

SensorBase::SensorBase(const String &identity): DeviceBase(identity) {
    this->capabilities = SensorCapabilities();
    this->resetSensorCapabilities();
}

SensorBase::~SensorBase() {}

bool SensorBase::isTimeToRun() {
    /*
    simple scheduler checking if the sensor must read sensor data this iteration
    */
    unsigned long now = millis();

    if (now - this->lastRun >= this->interval) {
        this->lastRun = now;
        return true;
    }
    return false;
}

unsigned long SensorBase::getSequence() {
    return this->sequence;
}

void SensorBase::resetSequence() {
    this->sequence = 0;
}

void SensorBase::appendMetaData(ModelBase &model) {
    if (this->capabilities.includeSequence) {
        model.sequence = this->sequence++;
    }
    
    if (this->capabilities.includeTimestamp) {
        model.timestamp = millis();
    }
}

String SensorBase::toJSON(ModelBase &model) {
    return model.toJSON(this->identity, this->capabilities.includeTimestamp, this->capabilities.includeSequence);
}

SensorCapabilities* SensorBase::getSensorCapabilties() {
    return &this->capabilities;
}

void SensorBase::setSensorCapabilities(SensorCapabilities* capabilities) {
    this->capabilities.enable = capabilities->enable;
    this->capabilities.includeTimestamp = capabilities->includeTimestamp;
    this->capabilities.includeSequence = capabilities->includeSequence;
    this->capabilities.sampleRate = capabilities->sampleRate;
    this->capabilities.dataOnStateChange = capabilities->dataOnStateChange;
    this->calculateInterval();
}

void SensorBase::resetSensorCapabilities() {
    this->capabilities.enable = DEVICE::DEFAULT_INCLUDE;
    this->capabilities.includeTimestamp = SENS::DEFAULT_INCLUDE_TIMESTAMP;
    this->capabilities.includeSequence = SENS::DEFAULT_INCLUDE_SEQUENCE;
    this->capabilities.sampleRate = SENS::DEFAULT_SAMPLE_RATE;
    this->capabilities.dataOnStateChange = SENS::DEFAULT_DATA_ON_CHANGE;
    this->sequence = 0;
    this->calculateInterval();
}

bool SensorBase::hasStateChanged(String &currentState) {
    if (!this->capabilities.dataOnStateChange) return true;

    return this->lastState != currentState;
}

void SensorBase::enable()
{
    this->capabilities.enable = true;
}

void SensorBase::disable()
{
    this->capabilities.enable = false;
}

bool SensorBase::isEnabled()
{
    return this->capabilities.enable;
}

void SensorBase::calculateInterval()
{
    this->interval = 1000 / this->capabilities.sampleRate;
}