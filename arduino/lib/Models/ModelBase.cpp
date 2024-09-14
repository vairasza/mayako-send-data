#include <ArduinoJson.h>

#include "ModelBase.h"

String ModelBase::toJSON(const String &identity, bool include_timestamp, bool include_sequence)
{
    JsonDocument doc;
    doc["identity"] = identity;

    this->appendModelData(doc);

    if (include_timestamp) {
        doc["timestamp"] = this->timestamp;
    }
    if (include_sequence) {
        doc["sequence"] = this->sequence;
    }

    String buffer;
    serializeJson(doc, buffer);

    return buffer;
}