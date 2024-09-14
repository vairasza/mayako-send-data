#ifndef LED_ACTUATOR_H
#define LED_ACTUATOR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

#include "ActuatorBase.h"

class LEDActuator: public ActuatorBase {
    public:
        LEDActuator(const String &identity, uint8_t pin, uint8_t numberOfPixels);
        ~LEDActuator();

        void identificationAction() override;
        void getCommandsDefinition(JsonArray& json) override;

        void switchOn(int index, uint8_t red, uint8_t green, uint8_t blue);
        void switchOff();

    private:
        uint8_t pin;
        uint8_t numberOfPixels;
        Adafruit_NeoPixel pixels;

};

#endif
