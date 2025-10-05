#pragma once

#include <Arduino.h>
#include <Ticker.h>

#include "utils.h"

typedef enum {
    LED_STATUS_IDLE,
    LED_STATUS_BLINK,
    LED_STATUS_FADING,
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
              int random_midpoint_percent = DEFAULT_RANDOM_MIDPOINT) : 
        name(name),
        _pin(pin), 
        _selected(false),
        _status(LED_STATUS_IDLE) {
        set_random_midpoint(random_midpoint_percent);
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


    void start_heartbeat(uint8_t interval_ms) {
        enable(false);
        _status = LED_STATUS_FADING;
        _fade_idx = FADE_MIN;
        Serial.printf("HBEAT IN %s %dms\n", name.c_str(), interval_ms);
        _ticker.attach_ms(interval_ms, +[] (LEDDevice* self) {
            self->_heartbeat();
        }, this);
    }



    void start_fade_in(uint8_t interval_ms) {
        enable(false);
        _status = LED_STATUS_FADING;
        _fade_idx = FADE_MIN;
        // interval_ms = _clamp((int)interval_ms, MIN_FADE_INTERVAL_MS, MAX_FADE_INTERVAL_MS);
        Serial.printf("FADE IN %s %dms\n", name.c_str(), interval_ms);
        _ticker.attach_ms(interval_ms, +[] (LEDDevice* self) {
            self->_fade_in();
        }, this);
    }

    void start_fade_out(uint8_t interval_ms) {
        enable(true);
        _status = LED_STATUS_FADING;
        _fade_idx = FADE_MAX;
        interval_ms = _clamp((int)interval_ms, MIN_FADE_INTERVAL_MS, MAX_FADE_INTERVAL_MS);
        Serial.printf("FADE OUT %s %dms\n", name.c_str(), interval_ms);        
        _ticker.attach_ms(interval_ms, +[] (LEDDevice* self) {
            self->_fade_out();
        }, this);
    }

    void enable(bool enabled) {
        _ticker.detach();
        _status = LED_STATUS_IDLE;
        // Serial.printf("ENABLE %s %s\n", name.c_str(), enabled ? "ON" : "OFF");
        digitalWrite(_pin, enabled ? HIGH : LOW);
    }


    static constexpr uint8_t _dim_tab[256] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  1,  1,  1,
        1,  1,  1,  1,  1,  2,  2,  2,  2,  3,
        3,  3,  4,  4,  5,  5,  6,  6,  7,  7,
        8,  9,  9, 10, 10, 11, 12, 13, 13, 14,
        15, 16, 17, 18, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 29, 30, 31, 32, 34, 35,
        36, 38, 39, 41, 42, 44, 45, 47, 49, 50,
        52, 54, 56, 58, 60, 62, 64, 66, 68, 70,
        72, 74, 76, 78, 81, 83, 85, 88, 90, 93,
        95, 98,101,103,106,109,111,114,117,120,
        123,126,129,132,135,138,141,145,148,151,
        155,158,162,165,169,172,176,180,184,188,
        191,195,199,203,207,211,215,219,223,227,
        231,235,239,243,247,251,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255
    };




    void dim(int value) {
        _ticker.detach();
        _status = LED_STATUS_IDLE;
        value *= 2;
        int analog_value = _dim_tab[value];
        Serial.printf("DIM %s %d an %d\n", name.c_str(), value, analog_value);
        analogWrite(_pin, analog_value);
    }

    void set_random_midpoint(int percent) {
        _random_midpoint = RAND_MAX / (100.0 / _clamp(percent, MIN_RANDOM_MIDPOINT, MAX_RANDOM_MIDPOINT));
        Serial.printf("RMID: %d%%\n", percent);
    }

private:
    uint8_t     _pin;
    bool        _selected;
    int         _random_midpoint;
    Ticker      _ticker;
    eLEDStatus  _status;
    uint16_t    _fade_idx = FADE_MIN;
   
    void _toggle() {
        digitalWrite(_pin, !digitalRead(_pin));
    }

    void _randomize() {
        bool enabled = rand() > _random_midpoint;
        digitalWrite(_pin, enabled ? LOW : HIGH);
    }

    void _fade_in() {
        // Serial.printf("idx %d val %d\n", _fade_idx, _dim_tab[_fade_idx]);
        analogWrite(_pin, _dim_tab[_fade_idx++]);
        if (_fade_idx == FADE_MAX) {
            // fade finished
            Serial.println(F("FADE IN ENDED"));
            _fade_idx = FADE_MIN;
            _status = LED_STATUS_IDLE;
            _ticker.detach();
        }
    }
    
    void _fade_out() {
        // Serial.println(duty);
        analogWrite(_pin, _dim_tab[_fade_idx--]);
        if (_fade_idx == FADE_MIN) {
            // fade finished
            Serial.println(F("FADE OUT ENDED"));
            _fade_idx = FADE_MAX;
            _status = LED_STATUS_IDLE;
            _ticker.detach();
        }
    }


    void _heartbeat() {
        static int delta = 1;
        _fade_idx += delta;
        Serial.printf("idx %d val %d\n", _fade_idx, _dim_tab[_fade_idx]);
        analogWrite(_pin, _dim_tab[_fade_idx]);
        if (_fade_idx == FADE_MAX && delta > 0) {
            // fade in finished
            Serial.println(F("HBEART REVERT"));
            delta = -1;
        }
        else if (_fade_idx == FADE_MIN && delta < 0) {
            // fade out finished
            Serial.println(F("HBEART REVERT"));
            delta = 1;
        }
    }

};
