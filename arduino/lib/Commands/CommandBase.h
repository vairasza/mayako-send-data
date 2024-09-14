#ifndef COMMAND_BASE_H
#define COMMAND_BASE_H

#include <ArduinoJson.h>

class CommandBase {
    public:
        //json must be a pointer so that we can use nullptr
        virtual void execute(JsonDocument *json) = 0;//datat is deserialised before, mapped to the command and transfered as jsondoc to fit to the method head
};

#endif