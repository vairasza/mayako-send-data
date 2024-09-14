#ifndef CAPABILITES_H
#define CAPABILITES_H

#include <Arduino.h>
#include <ArduinoJson.h>

struct SensorCapabilities {
    bool enable;//enables the sensor
    bool includeTimestamp; //sensor values include timestamp; although this setting is global, 
    bool includeSequence; //sensors values include sequence
    unsigned long sampleRate; //the interval at which sensor data is read
    bool dataOnStateChange;//only send data when the state of the sensors changes to the last time
};

struct ActuatorCapabilities {
    bool enable;//enables the sensor
};

struct DeviceCapabilities {//these are the devices general settings for a record
    unsigned long duration; //0 forever, 1+ keep sending data for x seconds
    unsigned long maxSamples; //samples data until the maxSamples value is reached. compares the with sensors value with the highest sample rate
    int delay; //seconds after using the start command until recording actually starts; example: can be used to move from the computer to the recording location where you have more freedom to perform gestures.
};

class ISensorCapabilities {
    public:
        virtual SensorCapabilities* getSensorCapabilties() = 0;//this also must include model props
        virtual void setSensorCapabilities(SensorCapabilities* capabilities) = 0;
        virtual void resetSensorCapabilities() = 0;
        virtual void getModelDefinition(JsonObject& json) = 0;
};

class IActuatorCapabilities {
    public:
        virtual ActuatorCapabilities* getActuatorCapabilties() = 0;
        virtual void setActuatorCapabilities(ActuatorCapabilities* capabilities) = 0;
        virtual void resetActuatorCapabilities() = 0;
        virtual void getCommandsDefinition(JsonArray& json) = 0;
};

class IDeviceCapabilities {
    public:
        virtual void resetCapabilities() = 0;
};

#endif