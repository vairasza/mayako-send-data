#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <memory>
#include <queue>
#include <vector>

#include "Packet.h"

//prefixes a logger messages with file name and line number
//code reference: https://gcc.gnu.org/onlinedocs/cpp/Standard-Predefined-Macros.html
#define prefix(message) (String("[") + String(__FILE__) + ":" + String(__LINE__) + "] " + String(message))

class Logger {
    public:
        //https://refactoring.guru/design-patterns/singleton/cpp/example
        static Logger* getInstance();

        void setQueue(std::queue<std::shared_ptr<Packet>> *queue);

        void error(String &message);
        void ferror(String &message, std::vector<String> valuesToReplace);
        void debug(String &message);
        void fdebug(String &message, std::vector<String> valuesToReplace);
                
        void enableDebugMode();
        void disableDebugMode();

    private:
        Logger();// Private constructor

        String format(String &message, std::vector<String> valuesToReplace);
        
        static Logger *instance; // Static instance pointer
        bool debugging;
        bool isEnabled;
        std::queue<std::shared_ptr<Packet>> *logQueue;
};

#endif
