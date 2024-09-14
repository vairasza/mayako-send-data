#ifndef MC_NAME
#define MC_NAME "PH00" //define a placeholder to work around undefined error with correct datatype but this will be replaced by variables defined in ArduinoUploader in mayako-core
#endif

#ifndef SERVICE_UUID
#define SERVICE_UUID "00000000-0000-0000-0000-000000000000"
#endif

#ifndef CHARACTERISTIC_UUID
#define CHARACTERISTIC_UUID "00000000-0000-0000-0000-000000000000"
#endif

#ifndef PORT
#define PORT "/dev/ttyUSB0"
#endif

#ifndef BAUDRATE
#define BAUDRATE 115200
#endif

#ifndef DEBUG_MODE
#define DEBUG_MODE 1
#endif

#ifndef WIRELESS_MODE
#define WIRELESS_MODE BLE
#endif

#define BLE 0
#define WIFI 1

#ifndef SSID
#define SSID "DefaultSSID"
#endif

#ifndef PASSWORD
#define PASSWORD "DefaultPassword"
#endif

#ifndef CLIENT_IP
#define CLIENT_IP "192.168.0.1"
#endif

#ifndef CLIENT_PORT
#define CLIENT_PORT 8080
#endif
