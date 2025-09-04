#pragma once

#include <Arduino.h>

#define TEST_BIT(v, b) (v & (1 << b))

class IDevice {

protected:
    const int _universe;
    const int _channel;
    const char* _name;

public:

    IDevice(const int universe, const int channel, const char* name) :
        _universe(universe),
        _channel(channel),
        _name(name) {
    }

    static IDevice* instance();
    
    virtual ~IDevice() {};

    // virtual int device_id() = 0;
    const int channel() { return _channel; };
    const int universe() { return _universe; };
    const char* name() const { return _name; };

    virtual void process(uint8_t * packet, uint16_t packet_len) = 0;


    void strobe(std::vector<uint8_t> pins, uint32_t duration_ms) {

        for (auto pin : pins) {
            Serial.printf("STROBE pin=%d %dms\n", pin, duration_ms);

        }
    }

    void fade(std::vector<uint8_t> pins, bool fade_in, uint32_t duration_ms) {
        for (auto pin : pins) {
            Serial.printf("FADE %s pin=%d %dms\n", fade_in ? "IN" : "OUT", pin, duration_ms);

        }
    }

    void random(std::vector<uint8_t> pins, uint32_t duration_ms) {
        for (auto pin : pins) {
            Serial.printf("RAND pin=%d %dms\n", pin, duration_ms);
        }
    }

    void enable(std::vector<uint8_t> pins, bool enabled) {
        for (auto pin : pins) {
            digitalWrite(pin, enabled ? HIGH : LOW);
        }
    }
};
