#ifndef DEVICE_BASE_H
#define DEVICE_BASE_H

#include <Arduino.h>

#include "Identification.h"
#include "Logger.h"

class DeviceBase: public IIdentification {
    public:
        DeviceBase(const String &identity);
        
        const String identity;
    
        //virtual void identificationAction() = 0;

    protected:
        Logger *logger;
};

#endif