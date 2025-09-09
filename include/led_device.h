#pragma once

#include <Arduino.h>
#include <Ticker.h>

class LEDDevice {
public:
    typedef enum {
        LED_STATUS_IDLE,
        LED_STATUS_BLINK,
        LED_STATUS_FADE_IN,
        LED_STATUS_FADE_OUT,
        LED_STATUS_RANDOM
    } eLEDStatus;   

private:
    uint8_t     _pin;
    bool        _enabled;
    Ticker      _ticker;
    eLEDStatus  _status;


public:
    LEDDevice(uint8_t pin) : 
        _pin(pin), 
        _enabled(false),
        _status(LED_STATUS_IDLE) {

        // default status OFF
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, HIGH);
    }


    static const unsigned long BLINK_REFRESH_MS = 100;
    static const unsigned long RANDOM_REFRESH_MS = 100;
    static const unsigned long FADE_DURATION_MS = 5000;

    #define FADE_DELAY_US   (int)((1'024'1000) / FADE_DURATION_MS)
    #define ANALOG_MAX      (1024)


    void enable(bool enabled) {
        set_status(LED_STATUS_IDLE);
        digitalWrite(_pin, !enabled);
    }


    void set_status(eLEDStatus status) {
        _status = status;
    }


    void tick() {
        switch(_status) {
            case LED_STATUS_IDLE: {
                // do nothing
            } break;

            case LED_STATUS_BLINK: {
                static unsigned long last_ts = 0;
                if (millis() - last_ts > BLINK_REFRESH_MS) {
                    _enabled = !_enabled;
                    enable(_enabled);
                    last_ts = millis();
                }
            } break;

            case LED_STATUS_RANDOM: {
                static unsigned long last_ts = 0;
                if (millis() - last_ts > RANDOM_REFRESH_MS) {
                    _enabled = rand() > (RAND_MAX / 2);
                    enable(_enabled);
                    last_ts = millis();
                }
            } break;

            case LED_STATUS_FADE_IN: {
                static uint16_t duty = 0;
                uint64_t last_ts = 0;
                if (micros64() - last_ts > FADE_DELAY_US) {
                    analogWrite(_pin, duty++);
                    if (duty == ANALOG_MAX) {
                        // fade finished
                        duty = 0;
                        _enabled = true;
                        _status = LED_STATUS_IDLE;
                    }
                    last_ts = micros64();
                }
            } break;

            case LED_STATUS_FADE_OUT: {
                static uint16_t duty = ANALOG_MAX;
                uint64_t last_ts = 0;
                if (micros64() - last_ts > FADE_DELAY_US) {
                    analogWrite(_pin, duty--);
                    if (duty == 0) {
                        // fade finished
                        duty = ANALOG_MAX;
                        _enabled = false;
                        _status = LED_STATUS_IDLE;
                    }
                    last_ts = micros64();
                }
            } break;

        }
    }


    // TODO: test these
    void start_blink(unsigned long intervalMs) {
        stop_blink(); // make sure any old timer is stopped
        _ticker.attach_ms(intervalMs, +[] (LEDDevice* self) {
            self->_toggle();
        }, this);
    }

    void stop_blink() {
        _ticker.detach();
        digitalWrite(_pin, HIGH);
        _enabled = false;
    }

    void start_random(unsigned long intervalMs) {
        stop_random(); // make sure any old timer is stopped
        _ticker.attach_ms(intervalMs, +[] (LEDDevice* self) {
            self->_randomize();
        }, this);
    }

    void stop_random() {
        _ticker.detach();
        digitalWrite(_pin, HIGH);
        _enabled = false;
    }

    void enable(bool enabled) {
        _ticker.detach();
        digitalWrite(_pin, !enabled);
        _enabled = false;
    }


    void _toggle() {
        _enabled = !_enabled;
        digitalWrite(_pin, _enabled);
    }

    void _randomize() {
        bool r = rand() > (RAND_MAX / 2);
        digitalWrite(_pin, r);
    }
};




class LEDManager {

    std::vector<LEDDevice> _leds;

    LEDDevice& at(int index) {
        return _leds.at(index);
    }

    void add_led(LEDDevice led) {
        _leds.push_back(led);
    }

    void tick() {
        for (LEDDevice led : _leds) {
            led.tick();
        }
    }
};