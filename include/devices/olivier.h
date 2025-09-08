#pragma once

#ifdef DEVICE_OLIVIER

#include "device.h"

#define PIN_FOREHEAD_STRIP  D1
#define PIN_RIGHT_EYE_LED   D2
#define PIN_LEFT_EYE_LED    D3
#define PIN_TEST_LED        LED_BUILTIN

class Device: public IDevice {

public:
    Device(int universe, int channel, const char* name, std::vector<uint8_t> pins)
        : IDevice(universe, channel, name, pins) {
    }

    void process(uint8_t * dmx_data, uint16_t dmx_data_len) {

        int pc = dmx_data[_channel];
        
        Serial.printf("%s process pc=%d\n", _name, pc);

        std::vector<uint8_t> pins;

        // bit 0: enable / disable
        bool enabled = TEST_BIT(pc, 0);

        // bit 1: forehead
        if (TEST_BIT(pc, 1)) {
            Serial.printf("forehead %s\n", enabled ? "ON" : "OFF");
            pins.push_back(PIN_FOREHEAD_STRIP);
        }

        // bit 2: left eye
        if (TEST_BIT(pc, 2)) {
            Serial.printf("left %s\n", enabled ? "ON" : "OFF");
            pins.push_back(PIN_LEFT_EYE_LED);
        }
        
        // bit 3: right eye
        if (TEST_BIT(pc, 3)) {
            Serial.printf("right %s\n", enabled ? "ON" : "OFF");
            pins.push_back(PIN_RIGHT_EYE_LED);
        }

        // bit 4: test
        if (TEST_BIT(pc, 4)) {
            Serial.printf("test %s\n", enabled ? "ON" : "OFF");
            pins.push_back(PIN_TEST_LED);
        }

        enable(pins, enabled);
    }
};

#endif

