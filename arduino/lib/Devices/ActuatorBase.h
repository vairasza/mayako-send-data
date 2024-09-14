#ifndef ACTUATOR_BASE_H
#define ACTUATOR_BASE_H

#include "DeviceBase.h"
#include "Capabilities.h"
#include "ActuatorCommand.h"

class ActuatorBase: public DeviceBase, public IActuatorCapabilities, public ActuatorCommand {
    public:
        ActuatorBase(const String& identity);
        ~ActuatorBase();

        ActuatorCapabilities* getActuatorCapabilties() override;
        void setActuatorCapabilities(ActuatorCapabilities* capabilities) override;
        void resetActuatorCapabilities() override;

    private:
        ActuatorCapabilities capabilities;
};

#endif