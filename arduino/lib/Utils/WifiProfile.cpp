#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

#include "WifiProfile.h"
#include "Storage.h"
#include "Config.h"
#include "Logger.h"
#include "Definitions.h"

/**
 * @class WiFiProfile
 * @cite https://github.com/vshymanskyy/Preferences
 * represents the WiFi credentials
 * queries the interval storage for WiFi credentials
 * creates, read, destroys profiles
 * there can be multiples profiles
 * there is one active profile which persists even after restart
 * keys are saved as a list
 * profiles are saved as a JSON string using the wifi_keys as keys
 * the active profile key is a String
 * the credentials are not valided if client_ip is a valid IP address etc.
 * there can be no credentials
 */
WiFiProfile::WiFiProfile(Storage *storage): storage(storage){
    this->logger = Logger::getInstance();

    this->processBuildProfile();
}

WiFiProfile::~WiFiProfile() {}

/**
 * @brief this function takes the build variables that are loosley defined in definitions.h and creates a default profile in storage
 */
void WiFiProfile::processBuildProfile()
{
    JsonDocument doc;
    doc["wifi_key"] = NET::PIMARY_WIFI_KEY;
    doc["ssid"] = SSID;
    doc["password"] = PASSWORD;
    doc["client_ip"] = CLIENT_IP;
    doc["client_port"] = CLIENT_PORT;

    String buffer;
    serializeJson(doc, buffer);

    this->storage->set(NET::PIMARY_WIFI_KEY, buffer);
}

/**
 * @brief reads the active profile
 * @note used in networkmanager for the command functions
 */
bool WiFiProfile::readActiveProfile(JsonDocument& json) {
    String activeKey = this->getActiveProfileKey();
    if (activeKey.isEmpty()) return false;

    String profile = this->storage->get(activeKey);
    if (profile.isEmpty()) return false;

    JsonDocument doc;
    deserializeJson(doc, profile);

    json["wifi_key"] = activeKey;
    json["ssid"] = doc["ssid"].as<String>();
    json["password"] = doc["password"].as<String>();
    json["client_ip"] = doc["client_ip"].as<String>();
    json["client_port"] = doc["client_port"].as<uint16_t>();

    return true;
}

/**
 * @brief create a new profile in the internal storage; this method also overwrites existing profiles, so be carefule that you do not use the keys from profiles that you want to use again
 * @note used in networkmanager for the command functions
 * @return if the opertation as successful
 */
bool WiFiProfile::createProfile(String key, char* profile) {
    if (key.isEmpty()) return false;

    //first we check if the key is already in the list of wifi keys to avoid doubles
    std::vector<String> keys = this->getWiFiProfileKeys();
    for (String item : keys) {
        if (item == key) return false;
    }

    bool success = this->storage->set(key, profile);

    return success == 1;
}

/**
 * @brief read a single profile using a String key. the result is added to the JsonDocument passed as reference.
 * @note used in networkmanager for the command functions
 */
bool WiFiProfile::readProfile(String key, JsonDocument& json) {
    if (key.isEmpty()) return false;

    String profile = this->storage->get(key);
    if (profile.isEmpty()) return false;

    JsonDocument profileDoc;
    deserializeJson(profileDoc, profile);

    json["ssid"] = profileDoc["ssid"];
    json["password"] = profileDoc["password"];
    json["client_ip"] = profileDoc["client_ip"];
    json["client_port"] = profileDoc["client_port"];

    return true;
}

/**
 * @brief reads all available profiles from the internal storage using the list of profile keys that are also from the internal strorage and stores it in a JsonObject so that the user can see all profile.
 * @note used in networkmanager for the command functions
 */
bool WiFiProfile::readAllProfiles(JsonArray& json) {
    std::vector<String> keys = this->getWiFiProfileKeys();
    if (keys.empty()) return false;

    for (String item: keys) {
        JsonObject dst = json.add<JsonObject>();
        String src = this->storage->get(item);
        if (src.isEmpty()) continue;
        
        JsonDocument profileDoc;
        deserializeJson(profileDoc, src);

        dst["wifi_key"] = item;
        dst["ssid"] = profileDoc["ssid"];
        dst["password"] = profileDoc["password"];
        dst["client_ip"] = profileDoc["client_ip"];
        dst["client_port"] = profileDoc["client_port"];
    }

    return true;
}

/**
 * @brief selects a wifi profile using a wifi key as the active wifi profile. This first ensures that there is a key and a profile in the interal storage. next, the key is set as active wifi key in the internal storage and the credentials are updated to reflect the changes. this function does not verify if the credentials are valid. We do not use NET::ACTIVE_WIFI_KEY because in this function we intend to change the active key. before selecting a profile, it first must be created.
 * @note used in networkmanager for the command functions
 * @return returns false if the key or profile does not exist and true if the key/profile was selected
 */
bool WiFiProfile::selectActiveProfile(String key) {
    if (key.isEmpty()) return false;

    //check if key is in list of wifi keys, otherwise return failure
    std::vector<String> data = this->getWiFiProfileKeys();
    if (data.empty()) return false;

    //check if the key is present in the list of wifi keys
    auto it = std::find(data.begin(), data.end(), key);
    if (it == data.end()) return false;

    //load the profile from the interval storage using the key from the arguments
    String profile = this->storage->get(key);
    if (profile.isEmpty()) return false;
    
    //from here on we know that the key exists and has an associated profile; 
    bool success = this->storage->set(NET::WIFI_ACTIVE_KEY, key);
    if (success == 0) return false; //could not set the key as the active wifi key in the internal storage

    //data is vector
    JsonDocument doc;
    deserializeJson(doc, profile);

    this->credentials.ssid = doc["ssid"].as<String>();
    this->credentials.password = doc["password"].as<String>();
    this->credentials.clientIp = doc["client_ip"].as<String>();
    this->credentials.clientPort = doc["client_port"].as<uint16_t>();

    this->hasActiveProfile = true;

    return true;
}

/**
 * @brief using a wifi_key removes the according profile and the corresponding key from the wifi_keys from the storage
 * 
 * removing all keys and profiles makes wifi unusable and requires another profile to be created with another protocol
 * @note used in networkmanager for the command functions
 * @return if both operations are successful
 */
bool WiFiProfile::destroyProfile(String key) {
    if (key.isEmpty()) return false;

    bool success = this->storage->destroy(key);

    if (!success) return false;

    success = this->destroyWiFiProfileKey(key);
    
    return success;
}

/**
 * @brief we load the profile from internal storage which matches the active wifi key and update the wifi profile credentials. this function is intended to be loaded in wifiprotocol before we create the wifi-udp object.
 * @return if there is an active profile
 */
bool WiFiProfile::loadActiveProfile() {
    //load the profile from the interval storage using the active wifi key
    String profile = this->storage->get(NET::WIFI_ACTIVE_KEY);
    if (profile.isEmpty()) return false;
    
    JsonDocument doc;
    deserializeJson(doc, profile);

    this->credentials.ssid = doc["ssid"].as<String>();
    this->credentials.password = doc["password"].as<String>();
    this->credentials.clientIp = doc["client_ip"].as<String>();
    this->credentials.clientPort = doc["client_port"].as<uint16_t>();

    this->hasActiveProfile = true;

    return true;
}

/**
 * @brief loads all wifi profile keys from the internal storage, deserlialises it from json to individual strings an returns it as a vector. this is the prerequisite to load the actual profiles from the storage unless you read the active profile or have a wifi key.
 * @return returns a vector with all wifi keys
 */
std::vector<String> WiFiProfile::getWiFiProfileKeys() {
    std::vector<String> wifiKeys;
    String keys = this->storage->get(NET::WIFI_KEYS);
    if (keys.isEmpty()) return wifiKeys;

    JsonDocument doc;
    deserializeJson(doc, keys);

    JsonArray data = doc["wifi_keys"];
    for (JsonVariant item : data) {
        wifiKeys.push_back(item.as<String>());
    }

    return wifiKeys;
}

/**
 * @brief adds a wifi key to the list of wifi keys in the internal storage
 * @return returns if the operation was successful
 */
bool WiFiProfile::setWiFiProfileKey(String key) {
    if (key.isEmpty()) return false;

    std::vector<String> wifiKeys = this->getWiFiProfileKeys();

    JsonDocument doc;
    JsonArray data = doc["wifi_keys"].to<JsonArray>();
    data.add(key);

    String buffer;
    serializeJson(doc, buffer);

    size_t success = this->storage->set(NET::WIFI_KEYS, buffer);

    return success == 1;
}

/**
 * @brief removes an item from the list of wifi keys in the internal storage
 * @return if the operation was successful
 */
bool WiFiProfile::destroyWiFiProfileKey(String key) {
    if (key.isEmpty()) return false;

    std::vector<String> wifiKeys = this->getWiFiProfileKeys();

    JsonDocument doc;
    JsonArray data = doc["wifi_keys"].to<JsonArray>();
    for (String item : wifiKeys) {
        if (item == key) continue;
        data.add(item);
    }

    String buffer;
    serializeJson(doc, buffer);

    bool success = this->storage->set(NET::WIFI_KEYS, buffer);

    return success;
}

/**
 * @brief set a key as the active wifi key; this key prepresents the currently used profile; to ensure that the profile is useable after restarting the device, we set this key in the storage as activeKey so
 * @return if the operation was successful
 */
bool WiFiProfile::setActiveProfileKey(String key) {
    if (key.isEmpty()) return false;

    size_t result = this->storage->set(NET::WIFI_ACTIVE_KEY, key);
    return result == 1;
}

/**
 * @return returns the wifi key which was last used (active wifi key)
 */
String WiFiProfile::getActiveProfileKey() {
    return this->storage->get(NET::WIFI_ACTIVE_KEY);
}
