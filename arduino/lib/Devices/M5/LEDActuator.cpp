#include <M5Stack.h>
#undef min
#include <Adafruit_NeoPixel.h>

#include "LEDActuator.h"
#include "Config.h"

LEDActuator::LEDActuator(const String &identity, uint8_t pin, uint8_t numberOfPixels): pin(pin), numberOfPixels(numberOfPixels), ActuatorBase(identity) {    
    this->pixels = Adafruit_NeoPixel(numberOfPixels, pin , NEO_GRB + NEO_KHZ800);
    this->addFunction(CUSTOM_CMD::SWITCH_ON, [this](JsonDocument& args) {
        int index = args["index"];
        uint8_t red = args["red"];
        uint8_t green = args["green"];
        uint8_t blue = args["blue"];
        
        this->switchOn(index, red, green, blue);        
    });

    this->addFunction(CUSTOM_CMD::SWITCH_OFF, [this](JsonDocument& args) {
        this->switchOff();
    });

    pixels.begin();
    pixels.clear();
    pixels.show();    
}

LEDActuator::~LEDActuator() {}

void LEDActuator::identificationAction() {
    //identify led actuator
}

void LEDActuator::switchOn(int index, uint8_t red, uint8_t green, uint8_t blue) {           
    uint32_t color = pixels.Color(red, green, blue);    
    pixels.setPixelColor(index, color);    
    pixels.show();        
}

void LEDActuator::switchOff() {    
    pixels.clear();
    pixels.show();    
}

void LEDActuator::getCommandsDefinition(JsonArray& json) {
    JsonDocument doc1;
    doc1["cmd_name"] = CUSTOM_CMD::SWITCH_ON;
    doc1["index"] = "int";
    doc1["red"] = "uint8_t";
    doc1["green"] = "uint8_t";
    doc1["blue"] = "uint8_t";

    json.add(doc1);

    JsonDocument doc2;
    doc2["cmd_name"] =  CUSTOM_CMD::SWITCH_OFF;

    json.add(doc2);
}