#ifndef MODEL_BASE_H
#define MODEL_BASE_H

#include <Arduino.h>
#include <ArduinoJson.h>

class ModelBase {
    public:
        ModelBase()
            : timestamp(0), sequence(0) {}

        unsigned long timestamp;
        unsigned long sequence;

        String toJSON(const String &identity, bool include_timestamp, bool include_sequence);
        virtual void getModelDefinition(JsonObject& json) = 0;

    protected:
        virtual void appendModelData(JsonDocument& ojb) = 0;

};

#endif