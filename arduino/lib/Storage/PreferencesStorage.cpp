#include <Preferences.h>
#include <Arduino.h>

#include "Storage.h"
#include "PreferencesStorage.h"

/**
 * @cite https://github.com/vshymanskyy/Preferences
 */
PreferencesStorage::PreferencesStorage(const String& deviceName): Storage() {
    this->deviceName = deviceName.c_str();
}

/**
 * @brief retrieves a value from the interval storage using a String key.
 * @return returns a String if there is a matching key-value pair in the storage otherweise returns an empty string
 */
String PreferencesStorage::get(String key) {
    //access to preferences is best opened/closed with each transaction unless to application is performance critical
    this->prefs.begin(this->deviceName, false);
    if (not this->prefs.isKey(key.c_str())) {
        return String();
    }

    auto result = this->prefs.getString(key.c_str());
    this->prefs.end();
    
    return result;
}

/**
 * @brief stores a key-value combination in the internal storage
 * @return returns 1 for successful operation, returns 0 for failed operation
 */
size_t PreferencesStorage::set(String key, String value) {
    this->prefs.begin(this->deviceName, false);
    size_t result = this->prefs.putString(key.c_str(), value);
    this->prefs.end();

    return result;
}

/**
 * @brief removes a key-value pair from the interval storage
 * @return returns 1 for successful operation, returns 0 for failed operation
 */
bool PreferencesStorage::destroy(String key) {
    this->prefs.begin(this->deviceName);
    bool success = this->prefs.remove(key.c_str());
    this->prefs.end();

    return success;
}