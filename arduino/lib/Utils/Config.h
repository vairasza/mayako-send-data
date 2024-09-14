#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <limits.h>

/* constants that are used to identify commands in the commandmanager class. */
namespace CMD {
    const String RECORD_CREATE = "RECORD_CREATE";
    const String RECORD_START = "RECORD_START";
    const String RECORD_STOP = "RECORD_STOP";
    const String RECORD_READ = "RECORD_READ";
    const String RESTART = "RESTART";
    const String BATTERY_READ = "BATTERY_READ";
    const String IDENTIFY = "IDENTIFY";

    const String CONNECTION_READ = "CONNECTION_READ";
    const String WIFI_PROFILE_CREATE = "WIFI_PROFILE_CREATE";
    const String WIFI_PROFILE_READ = "WIFI_PROFILE_READ";
    const String WIFI_PROFILE_ACTIVE_READ = "WIFI_PROFILE_ACTIVE_READ";
    const String WIFI_PROFILE_ALL_READ = "WIFI_PROFILE_ALL_READ";
    const String WIFI_PROFILE_ACTIVE_SELECT = "WIFI_PROFILE_ACTIVE_SELECT";
    const String WIFI_PROFILE_DELETE = "WIFI_PROFILE_DELETE";
    const String SELECT_BLE = "SELECT_BLE";
    const String SELECT_WIFI = "SELECT_WIFI";
    const String SELECT_SERIAL = "SELECT_SERIAL";
    const String ACKNOWLEDGEMENT_ENABLE = "ACKNOWLEDGEMENT_ENABLE";
    const String ACKNOWLEDGEMENT_DISABLE = "ACKNOWLEDGEMENT_DISABLE";
}

namespace CUSTOM_CMD {
    const String SWITCH_ON = "SWITCH_ON";
    const String SWITCH_OFF = "SWITCH_OFF";
}

namespace PIN {
    /**
     * These pins are the ports on the outside of the device
     * @cite https://community.m5stack.com/post/1940
    */
    namespace M5 {
        const uint8_t PORT_A = 21; //red port; G21
        const uint8_t PORT_B = 26; //black port; G26
        const uint8_t PORT_C = 17; //blue port; G16
    }
}

namespace BUTT {   
    const String ON_A = "BUTTON_A";
    const String ON_B = "BUTTON_B";
    const String ON_C = "BUTTON_C";
}

/* contants used in the storage class to create a namespace in the preferences storage. */
namespace NET {
    const String WIFI_KEYS = "WIFI_KEYS";
    const String WIFI_ACTIVE_KEY = "WIFI_ACTIVE_KEY";
    const String PIMARY_WIFI_KEY = "PIMARY_WIFI_KEY";
    const uint16_t MAX_BUFFER_SIZE = 512;
    const String SERIAL_NAME = "SERIAL";
    const String WIFI_NAME = "WIFI";
    const String BLE_NAME = "BLE";
    const bool IS_CONNECTED_DEFAULT = false;
    const int TIME_TO_CONNECT_PROTOCOL = 500;
    const int TIMEOUT_WIRELESS_UPGRADE = 1000;
    const int TIMEOUT_DEFAULT = 50;
    const size_t OUT_OF_ORDER_PACKET_MAX_SIZE = 5;
    const uint16_t SEQUENCE_MAX_NUMBER_SIZE = std::numeric_limits<uint16_t>::max();
    const bool SEND_ACK_PACKETS = false;
    const unsigned int HEARTBEAT_INTERVAL = 1000;
    const int BLE_ATT_OVERHEAD = 3;
    const int BLE_EXPECTED_MTU = 256;
    const int BLE_CHUNK_TIMEOUT = 5;

    namespace HEADER {
        const size_t SIZE = 10;
        const unsigned int PAYLOADSIZE_POSITION = 8;
        //reserve the 6 characters in ASCII table starting with 0x20 and ending with 0x26 as the beginning byte of our packets
        const uint8_t METHOD_ACKNOWLEDGEMENT = 0x20; //SP
        const uint8_t METHOD_DATA = 0x21; //!
        const uint8_t METHOD_COMMAND = 0x22; //"
        const uint8_t METHOD_HEARTBEAT = 0x23; //#
        const uint8_t METHOD_DEBUG = 0x24; //$
        const uint8_t METHOD_INFO = 0x25; //%
        const uint8_t METHOD_ERROR = 0x26; //&
    }
}

namespace DEVICE {
    const unsigned long DURATION = 0;//milli seconds
    const unsigned long MAX_SAMPLES = 0;
    const int DELAY = 0;//milli seconds
    const bool DEFAULT_INCLUDE = true;//if a sensor/actuator is enabled
}

namespace MC {
    const size_t MC_NAME_LENGTH = 4;
    const int32_t LOOP_WAIT_TIME_DEFAULT = 5;
}

namespace SENS {
    const bool DEFAULT_INCLUDE_SEQUENCE = false;
    const bool DEFAULT_INCLUDE_TIMESTAMP = false;
    const int DEFAULT_SAMPLE_RATE = 10;
    const bool DEFAULT_DATA_ON_CHANGE = false;
}

namespace ACT {
    const bool DEFAULT_INCLUDE_ACTUATOR = false;
    
    namespace SK6812 {
        const uint8_t NUM_PIXELS = 3;
    }
}

namespace UTIL {
    const String REPLACE_STRING = "%%";
    #if DEBUG_MODE
        const bool LOGGER_DEBUGGING_DEFAULT = true; 
    #else
        const bool LOGGER_DEBUGGING_DEFAULT = false;
    #endif
}

#endif