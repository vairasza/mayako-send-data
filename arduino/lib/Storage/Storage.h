#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>

class Storage {
    public:
        //Storage();
        virtual String get(String key) = 0;
        virtual size_t set(String key, String value) = 0;
        virtual bool destroy(String key) = 0;     
};

#endif