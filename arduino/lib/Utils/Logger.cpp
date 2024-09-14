#include <Arduino.h>
#include <ArduinoJson.h>
#include <queue>
#include <vector>
#include <memory>

#include "Logger.h"
#include "Packet.h"
#include "Config.h"

Logger *Logger::instance = nullptr;

Logger::Logger(): logQueue(nullptr), debugging(UTIL::LOGGER_DEBUGGING_DEFAULT) {}

Logger *Logger::getInstance() {
    if (instance == nullptr) {
        instance = new Logger();
    }

    return instance;
}

void Logger::setQueue(std::queue<std::shared_ptr<Packet>> *queue) {
    /* we only want the queue from the networkmanager so that all logs are directed there; overriding the queue would lead to logs being lost. */
    if (this->logQueue == nullptr) {
        this->logQueue = queue;
    }
}

void Logger::error(String &message) {
    if (this->logQueue == nullptr) return;

    JsonDocument doc;
    doc["error"] = message;

    size_t bufferSize = measureJson(doc) + 1;
    char buffer[bufferSize];
    serializeJson(doc, buffer, bufferSize);
    
    std::shared_ptr<Packet> packet = std::make_shared<Packet>();
    packet->setMethod(NET::HEADER::METHOD_ERROR);
    packet->setPayload(buffer);

    this->logQueue->push(std::move(packet));
}

void Logger::ferror(String &message, std::vector<String> valuesToReplace) {
    if (this->logQueue == nullptr) return;

    String formatedMessage = this->format(message, valuesToReplace);
    
    this->error(formatedMessage);
}

void Logger::debug(String &message) {
    //filter debug logs in production mode and if disabled in debug mode
    //debug mode can be setup in platform.ini
    if (this->logQueue == nullptr || !this->debugging) return;
    JsonDocument doc;
    doc["debug"] = message;

    size_t bufferSize = measureJson(doc) + 1;
    char buffer[bufferSize];
    serializeJson(doc, buffer, bufferSize);
    
    std::shared_ptr<Packet> packet = std::make_shared<Packet>();  
    packet->setMethod(NET::HEADER::METHOD_DEBUG);
    packet->setPayload(buffer);
  
    this->logQueue->push(std::move(packet));
}

void Logger::fdebug(String &message, std::vector<String> valuesToReplace) {
    if (this->logQueue == nullptr || !this->debugging) return;

    String formatedMessage = this->format(message, valuesToReplace);

    this->debug(formatedMessage);
}

String Logger::format(String &message, std::vector<String> valuesToReplace) {
    /*
    This function is an option to the log function but implements formating the string. It uses %% as an indicator to replace values provided through a vector. as long as the message contains %%, flog tries to replace is with valuesToReplace.
    */
    String formatedMessage = message;
    int index = 0;

    for (const String &value : valuesToReplace) {
        index = formatedMessage.indexOf(UTIL::REPLACE_STRING, index);
        if (index == -1) break;
        formatedMessage = formatedMessage.substring(0, index) + value + formatedMessage.substring(index + 2);
    }

    return formatedMessage;
}

void Logger::enableDebugMode() {
    this->debugging = true;
}

void Logger::disableDebugMode() {
    this->debugging = false;
}
