#ifndef SDCARD_STORAGE_H
#define SDCARD_STORAGE_H

#include <M5Stack.h>
#undef min

#include "Storage.h"

class SDCardStorage: public Storage {
    public:
        SDCardStorage();

        String get(String key) override;
        size_t set(String key, String value) override;
        bool destroy(String key) override;
        
    private:
        const char* nameSpace;
};

#endif