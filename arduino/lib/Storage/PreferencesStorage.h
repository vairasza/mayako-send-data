#ifndef PREFERENCES_STORAGE_H
#define PREFERENCES_STORAGE_H

#include <Preferences.h>
#include <Arduino.h>

#include "Storage.h"

class PreferencesStorage: public Storage {
    public:
        PreferencesStorage(const String& deviceName);

        String get(String key);
        size_t set(String key, String value);
        bool destroy(String key);
        
    private:
        Preferences prefs;
        const char* deviceName;
};

#endif