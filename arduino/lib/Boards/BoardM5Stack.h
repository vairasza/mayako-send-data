#ifndef BORAD_M5Stack_H
#define BORAD_M5Stack_H

#include "BoardBase.h"
#include "Logger.h"

class BoardM5Stack: public BoardBase {
    public:
        BoardM5Stack();
        void init() override;
        int getBattery() override;
        bool getBatteryCharging() override;
        void restart() override;
        void update() override;
        size_t getAllocatedHeap() override;
        void identify() override;

    private:
        Logger *logger;
};

#endif
