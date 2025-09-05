#pragma once

#ifdef DEVICE_JEROME

#include "device.h"

#define PIN_EYES_LED   D1

class Device: public IDevice {

public:
    Device(int universe, int channel, const char* name)
        : IDevice(universe, channel, name) {
        
        pinMode(PIN_EYES_LED, OUTPUT);
    }

    void process(uint8_t * dmx_data, uint16_t dmx_data_len) {

        int pc = dmx_data[_channel];
        
        Serial.printf("%s process pc=%d\n", _name, pc);

        std::vector<uint8_t> pins;

        // bit 0: enable / disable
        bool enabled = TEST_BIT(pc, 0);

        // bit 1: eyes
        if (TEST_BIT(pc, 1)) {
            Serial.printf("eyes %s\n", enabled ? "ON" : "OFF");
            pins.push_back(PIN_EYES_LED);
        }

        enable(pins, enabled);
    }
};

#endif
