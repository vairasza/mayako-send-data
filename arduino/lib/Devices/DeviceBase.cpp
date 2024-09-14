#include "Config.h"
#include "DeviceBase.h"

DeviceBase::DeviceBase(const String &identity): identity(identity) {
    this->logger = Logger::getInstance();
}
