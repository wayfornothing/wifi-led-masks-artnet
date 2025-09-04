#pragma once

#include <Arduino.h>
// #include <ESP8266WiFi.h>

// #define LED_DEVICE_ID(x)    (0xf0 & x)

// command for 1 device: 1 command byte + 3 bytes
typedef enum ECommand {
    ECommandEnable,  
    ECommandFadeIn,  
    ECommandFadeOut,
    ECommandStrobe,
    ECommandRandom,
    ECommandLast,
} ECommand;

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

    virtual bool process() = 0;


    // virtual bool process_packet(uint8_t *data, size_t length) = 0;
    // virtual void on() = 0;
    // virtual void off() = 0;

    // virtual void test() = 0;
};
