#pragma once

#ifdef DEVICE_MAXIME

#include "device.h"


#define PIN_RIGHT_EYE_LED   D1
#define PIN_LEFT_EYE_LED    D2

class Device: public IDevice {

public:
    Device(int universe, int channel, const char* name)
        : IDevice(universe, channel, name) {
        
        pinMode(PIN_RIGHT_EYE_LED, OUTPUT);
        pinMode(PIN_LEFT_EYE_LED, OUTPUT);
    }

    void process(uint8_t * dmx_data, uint16_t dmx_data_len) {

        int pc = dmx_data[_channel];
        
        Serial.printf("%s process pc=%d\n", _name, pc);

        std::vector<uint8_t> pins;

        // bit 0: enable / disable
        bool enabled = TEST_BIT(pc, 0);

        // bit 1: left arm
        if (TEST_BIT(pc, 1)) {
            Serial.printf("left eye %s\n", enabled ? "ON" : "OFF");
            pins.push_back(PIN_LEFT_EYE_LED);
        }

        // bit 2: right arm eye
        if (TEST_BIT(pc, 2)) {
            Serial.printf("right eye %s\n", enabled ? "ON" : "OFF");
            pins.push_back(PIN_RIGHT_EYE_LED);
        }

        enable(pins, enabled);
    }
};

#endif
