#pragma once

#ifdef DEVICE_JEROME

#include "device.h"

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