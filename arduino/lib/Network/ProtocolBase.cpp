#include "ProtocolBase.h"
#include "Config.h"

ProtocolBase::ProtocolBase(String name, uint16_t bufferSize): name(name), bufferSize(bufferSize) {}

String ProtocolBase::getName() {
    return this->name;
}