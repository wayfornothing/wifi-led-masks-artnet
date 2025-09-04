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
        : IDevice(universe, channel, name) {}

    bool process() {
        // TODO: do stuff

        Serial.printf("%s process\n", _name);

        return true;
    }
};

#endif
