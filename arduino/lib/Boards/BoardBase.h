#ifndef BOARD_BASE_H
#define BOARD_BASE_H

class BoardBase {
    public:
        virtual void init() = 0; //init, power, wire, speaker, imu, etc.  
        virtual int getBattery() = 0; //M5.Power.getBatteryLevel
        virtual bool getBatteryCharging() = 0;
        virtual void restart() = 0;
        virtual void update() = 0; //to update the sensors, etc.
        virtual size_t getAllocatedHeap() = 0; //returns the remaining memory in bytes
        virtual void identify() = 0;
};

#endif
