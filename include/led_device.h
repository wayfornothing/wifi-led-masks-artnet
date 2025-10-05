#pragma once

#include <Arduino.h>
#include <Ticker.h>

#include "utils.h"

typedef enum {
    LED_STATUS_IDLE,
    LED_STATUS_BLINK,
    LED_STATUS_FADE_IN,
    LED_STATUS_FADE_OUT,
    LED_STATUS_RANDOM
} eLEDStatus;   

// TODO: config thru web, this depends on LED type
#define FADE_MIN (0)
#define FADE_MAX (0xff)

static const int MIN_BLINK_INTERVAL_MS = 5;
static const int BLINK_MULTIPLIER = 1;
static const int DEFAULT_BLINK_INTERVAL_MS = 50;
static const int MAX_BLINK_INTERVAL_MS = 127 * 5;

static const int MIN_RANDOM_INTERVAL_MS = 5;
static const int RANDOM_MULTIPLIER = 1;
static const int DEFAULT_RANDOM_INTERVAL_MS = 50;
static const int MAX_RANDOM_INTERVAL_MS = 200;

static const int MIN_RANDOM_MIDPOINT = 1;
static const int DEFAULT_RANDOM_MIDPOINT = 50;
static const int MAX_RANDOM_MIDPOINT = 99;

static const int MIN_FADE_INTERVAL_MS = 1;
static const int DEFAULT_FADE_INTERVAL_MS = 2;
static const int MAX_FADE_INTERVAL_MS = 50;

class LEDDevice {
public:
    
    String name;

    LEDDevice(uint8_t pin, 
              String name, 
            //   int blink_interval_ms = DEFAULT_BLINK_INTERVAL_MS, 
            //   int random_interval_ms = DEFAULT_RANDOM_INTERVAL_MS, 
              int random_midpoint_percent = DEFAULT_RANDOM_MIDPOINT) : 
        name(name),
        _pin(pin), 
        _selected(false),
        _status(LED_STATUS_IDLE) {
        // set_blink_interval_ms(blink_interval_ms);
        // set_random_interval_ms(random_interval_ms);
        set_random_midpoint(random_midpoint_percent);
        // set_fade_interval_ms(fade_interval_ms);
        pinMode(_pin, OUTPUT);
        Serial.printf("Created LED %s at pin %d\n", name.c_str(), pin);
    }

    void select(bool select) {
        _selected = select;
    }

    bool is_selected() {
        return _selected;
    }

    void start_blink(uint8_t interval_ms) {
        interval_ms = interval_ms * BLINK_MULTIPLIER;
        interval_ms = _clamp((int)interval_ms, MIN_BLINK_INTERVAL_MS, MAX_BLINK_INTERVAL_MS);
        Serial.printf("BLINK %s %dms\n", name.c_str(), interval_ms);
        _status = LED_STATUS_BLINK;
        _ticker.detach();
        _ticker.attach_ms(interval_ms, +[] (LEDDevice* self) {
            // Serial.print(".");
            self->_toggle();
        }, this);
    }


    void start_random(uint8_t interval_ms) {
        Serial.println(_pin);
        interval_ms = interval_ms * RANDOM_MULTIPLIER;
        interval_ms = _clamp((int)interval_ms, MIN_RANDOM_INTERVAL_MS, MAX_RANDOM_INTERVAL_MS);
        Serial.printf("RANDOM %s %dms\n", name.c_str(), interval_ms);
        _status = LED_STATUS_RANDOM;
        _ticker.detach();
        _ticker.attach_ms(interval_ms, +[] (LEDDevice* self) {
            self->_randomize();
        }, this);
    }

    void start_fade_in(uint8_t interval_ms) {
        enable(false);
        _status = LED_STATUS_FADE_IN;
        _duty = FADE_MAX;
        interval_ms = _clamp((int)interval_ms, MIN_FADE_INTERVAL_MS, MAX_FADE_INTERVAL_MS);
        Serial.printf("FADE IN %s %dms\n", name.c_str(), interval_ms);
        _ticker.attach_ms(interval_ms, +[] (LEDDevice* self) {
            self->_fade_in();
        }, this);
    }

    void start_fade_out(uint8_t interval_ms) {
        enable(true);
        _status = LED_STATUS_FADE_OUT;
        _duty = FADE_MIN;
        interval_ms = _clamp((int)interval_ms, MIN_FADE_INTERVAL_MS, MAX_FADE_INTERVAL_MS);
        Serial.printf("FADE OUT %s %dms\n", name.c_str(), interval_ms);        
        _ticker.attach_ms(interval_ms, +[] (LEDDevice* self) {
            self->_fade_out();
        }, this);
    }

    void enable(bool enabled) {
        _ticker.detach();
        _status = LED_STATUS_IDLE;
        Serial.printf("ENABLE %s %s\n", name.c_str(), enabled ? "ON" : "OFF");
        digitalWrite(_pin, enabled ? HIGH : LOW);
    }

    void dim(int percent) {
        _ticker.detach();
        _status = LED_STATUS_IDLE;
        percent = _clamp(percent, 0, 100);
        Serial.printf("LED %s (pin %d) %d%%\n", name.c_str(), _pin, percent);
        int analog_value = map(percent, 0, 100, FADE_MIN, FADE_MAX);
        analogWrite(_pin, analog_value);
    }

    // void set_blink_interval_ms(int ms) {
    //     _blink_interval_ms = _clamp(ms, MIN_BLINK_INTERVAL_MS, MAX_BLINK_INTERVAL_MS);
    //     Serial.printf("BITV: %dms\n", _blink_interval_ms);
    // }

    // void inc_blink_interval_ms(int ms) {
    //     _blink_interval_ms = _clamp(_blink_interval_ms + ms, MIN_BLINK_INTERVAL_MS, MAX_BLINK_INTERVAL_MS);
    //     Serial.printf("BITV: %dms\n", _blink_interval_ms);
    // }

    // void set_random_interval_ms(int ms) {
    //     _random_interval_ms = _clamp(ms, MIN_RANDOM_INTERVAL_MS, MAX_RANDOM_INTERVAL_MS);
    //     Serial.printf("RITV: %dms\n", _random_interval_ms);
    // }

    void set_random_midpoint(int percent) {
        _random_midpoint = RAND_MAX / (100.0 / _clamp(percent, MIN_RANDOM_MIDPOINT, MAX_RANDOM_MIDPOINT));
        Serial.printf("RMID: %d%%\n", percent);
    }

    // void inc_random_interval_ms(int ms) {
    //     _random_interval_ms = _clamp(_random_interval_ms + ms, MIN_RANDOM_INTERVAL_MS, MAX_RANDOM_INTERVAL_MS);
    //     Serial.printf("RITV: %dms\n", _random_interval_ms);
    // }

    // void set_fade_interval_ms(int ms) {
    //     _fade_interval_ms = _clamp(ms, MIN_FADE_INTERVAL_MS, MAX_FADE_INTERVAL_MS);
    //     Serial.printf("FITV: %dms\n", _fade_interval_ms);
    // }

    // void inc_fade_interval_ms(int ms) {
    //     _fade_interval_ms = _clamp(_fade_interval_ms + ms, MIN_FADE_INTERVAL_MS, MAX_FADE_INTERVAL_MS);
    //     Serial.printf("FITV: %dms\n", _fade_interval_ms);
    // }

private:
    uint8_t     _pin;
    bool        _selected;
    // int         _blink_interval_ms;
    // int         _random_interval_ms;
    int         _random_midpoint;
    // int         _fade_interval_ms;
    Ticker      _ticker;
    eLEDStatus  _status;
    uint16_t    _duty = FADE_MIN;
   
    void _toggle() {
        digitalWrite(_pin, !digitalRead(_pin));
    }

    void _randomize() {
        bool enabled = rand() > _random_midpoint;
        digitalWrite(_pin, enabled ? LOW : HIGH);
    }

    void _fade_in() {
        // Serial.println(duty);
        analogWrite(_pin, _duty++);
        if (_duty == FADE_MAX) {
            // fade finished
            Serial.println(F("FADE IN ENDED"));
            _duty = FADE_MIN;
            _status = LED_STATUS_IDLE;
            _ticker.detach();
        }
    }
    
    void _fade_out() {
        // Serial.println(duty);
        analogWrite(_pin, _duty--);
        if (_duty == FADE_MIN) {
            // fade finished
            Serial.println(F("FADE OUT ENDED"));
            _duty = FADE_MAX;
            _status = LED_STATUS_IDLE;
            _ticker.detach();
        }
    }
};
