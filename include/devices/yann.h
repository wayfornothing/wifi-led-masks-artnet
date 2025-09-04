#pragma once

#ifdef DEVICE_YANN

#include "device.h"

#define PIN_INNER_LEFT_LED  D1
#define PIN_OUTER_LEFT_LED  D2
#define PIN_INNER_RIGHT_LED D3
#define PIN_OUTER_RIGHT_LED D4

class Device: public IDevice {

public:
    Device(int universe, int channel, const char* name)
        : IDevice(universe, channel, name) {

        pinMode(PIN_INNER_LEFT_LED, OUTPUT);
        pinMode(PIN_INNER_RIGHT_LED, OUTPUT);
        pinMode(PIN_OUTER_LEFT_LED, OUTPUT);
        pinMode(PIN_OUTER_RIGHT_LED, OUTPUT);
    }

    void process(uint8_t * dmx_data, uint16_t dmx_data_len) {

        int pc = dmx_data[_channel];
        
        Serial.printf("%s process pc=%d\n", _name, pc);

        std::vector<uint8_t> pins;

        // bit 0: enable / disable
        bool enabled = TEST_BIT(pc, 0);

        // bit 1: inner left
        if (TEST_BIT(pc, 1)) {
            Serial.printf("inner left %s\n", enabled ? "ON" : "OFF");
            pins.push_back(PIN_INNER_LEFT_LED);
        }

        // bit 2: outer left
        if (TEST_BIT(pc, 2)) {
            Serial.printf("outer left %s\n", enabled ? "ON" : "OFF");
            pins.push_back(PIN_OUTER_LEFT_LED);
        }
        
        // bit 3: inner right
        if (TEST_BIT(pc, 3)) {
            Serial.printf("inner right %s\n", enabled ? "ON" : "OFF");
            pins.push_back(PIN_INNER_RIGHT_LED);
        }
        
        // bit 4: outer right
        if (TEST_BIT(pc, 4)) {
            Serial.printf("outer right %s\n", enabled ? "ON" : "OFF");
            pins.push_back(PIN_OUTER_RIGHT_LED);
        }

        enable(pins, enabled);
    }
};

#endif
