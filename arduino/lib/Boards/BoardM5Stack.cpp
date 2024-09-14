#include <M5Stack.h>
#undef min
#include <esp_heap_caps.h>

#include "BoardM5Stack.h"
#include "BoardBase.h"
#include "Config.h"
#include "Logger.h"

BoardM5Stack::BoardM5Stack(): BoardBase() {}

void BoardM5Stack::init() {
    this->logger = Logger::getInstance();
    M5.begin();
    M5.Power.begin();
    M5.Speaker.end();
    M5.IMU.Init();
    this->logger->debug(prefix("board initiated"));
}

int BoardM5Stack::getBattery() {
    return M5.Power.getBatteryLevel();
}

bool BoardM5Stack::getBatteryCharging() {
    return M5.Power.isCharging();
}

void BoardM5Stack::restart() {
    M5.Power.reset();
}

void BoardM5Stack::update() {
    M5.update();
}

/**
 * @brief returns the number of bytes available on the heap
 * @cite https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/mem_alloc.html
 * @cite https://forum.arduino.cc/t/how-to-compute-the-total-size-of-ram-and-the-usage-size-for-esp32/929955/3
 * @return number of free bytes on the heap
 */
size_t BoardM5Stack::getAllocatedHeap() {
    size_t totalMemory = heap_caps_get_total_size(MALLOC_CAP_8BIT);
    size_t freeMemory = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    size_t usedMemory = totalMemory - freeMemory;

    return (usedMemory * 100) / totalMemory;
}

void BoardM5Stack::identify() {
    M5.Lcd.setBrightness(255);
    M5.Lcd.fillScreen(TFT_RED);
    M5.Speaker.beep();
    delay(500);
    M5.Lcd.fillScreen(TFT_GREEN);
    M5.Speaker.beep();
    delay(500);
    M5.Lcd.fillScreen(TFT_BLUE);
    M5.Speaker.beep();
    delay(500);
    M5.Lcd.setBrightness(0);
    M5.Speaker.mute();
}

//TODO: offer functions to enable speaker and move it out of the init function
