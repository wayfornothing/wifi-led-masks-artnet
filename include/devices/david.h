#pragma once

#ifdef DEVICE_DAVID

#include "device.h"

class Device: public IDevice {

public:
    Device(int universe, int channel, const char* name)
        : IDevice(universe, channel, name) {}

    void process(uint8_t * dmx_data, uint16_t dmx_data_len) {

        // TODO: do stuff
        Serial.printf("%s process\n", _name);
    }
};

#endif