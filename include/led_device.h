#pragma once

#include "hal/hal.h"
#include "utils.h"

#define FADE_MIN (0)
#define FADE_MAX (0xff)

static const int MIN_BLINK_INTERVAL_MS = 5;
static const int DEFAULT_BLINK_INTERVAL_MS = 50;
static const int MAX_BLINK_INTERVAL_MS = 127 * 5;

static const int MIN_RANDOM_INTERVAL_MS = 5;
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
              int random_midpoint_percent = DEFAULT_RANDOM_MIDPOINT,
              int heartbeat_max = FADE_MAX) : 
        name(name),
        _pin(pin), 
        _selected(false) {
        set_random_midpoint(random_midpoint_percent);
        set_heartbeat_max(heartbeat_max);
        pin_set_output(_pin);
        Logger::info("Created LED '%s' at pin '%s' (%d)", name.c_str(), string_from_pin(pin), pin);
    }

    void select(bool select) {
        // Logger::info("%sSELECT LED '%s' at pin '%s'", select ? "" : "UN", name.c_str(), string_from_pin(_pin));
        _selected = select;
    }

    bool is_selected() {
        return _selected;
    }

    void blink(uint8_t interval_ms) {
        interval_ms = interval_ms;
        interval_ms = _clamp((int)interval_ms, MIN_BLINK_INTERVAL_MS, MAX_BLINK_INTERVAL_MS);
        Logger::info("BLINK %s %dms", name.c_str(), interval_ms);
        _ticker.detach();
        _ticker.attach_ms(interval_ms, +[] (LEDDevice* self) {
            self->_toggle();
        }, this);
    }


    void random(uint8_t interval_ms) {
        interval_ms = interval_ms;
        interval_ms = _clamp((int)interval_ms, MIN_RANDOM_INTERVAL_MS, MAX_RANDOM_INTERVAL_MS);
        Logger::info("RANDOM %s %dms", name.c_str(), interval_ms);
        _ticker.detach();
        _ticker.attach_ms(interval_ms, +[] (LEDDevice* self) {
            self->_randomize();
        }, this);
    }


    void heartbeat(uint8_t interval_ms) {
        enable(false);
        _fade_idx = FADE_MIN;
        _heartbeat_delta = 1;
        Logger::info("HBEAT %s %dms", name.c_str(), interval_ms);
        _ticker.attach_ms(interval_ms, +[] (LEDDevice* self) {
            self->_heartbeat(true);
        }, this);
    }


    void pulse(uint8_t interval_ms) {
        enable(false);
        _fade_idx = FADE_MIN;
        _heartbeat_delta = 1;
        Logger::info("PULSE %s %dms", name.c_str(), interval_ms);
        _ticker.attach_ms(interval_ms, +[] (LEDDevice* self) {
            self->_heartbeat(false);
        }, this);
    }


    void fade_in(uint8_t interval_ms) {
        enable(false);
        _fade_idx = FADE_MIN;
        // interval_ms = _clamp((int)interval_ms, MIN_FADE_INTERVAL_MS, MAX_FADE_INTERVAL_MS);
        Logger::info("FADE IN %s %dms", name.c_str(), interval_ms);
        _ticker.attach_ms(interval_ms, +[] (LEDDevice* self) {
            self->_fade_in();
        }, this);
    }

    void fade_out(uint8_t interval_ms) {
        enable(true);
        _fade_idx = FADE_MAX;
        interval_ms = _clamp((int)interval_ms, MIN_FADE_INTERVAL_MS, MAX_FADE_INTERVAL_MS);
        Logger::info("FADE OUT %s %dms", name.c_str(), interval_ms);        
        _ticker.attach_ms(interval_ms, +[] (LEDDevice* self) {
            self->_fade_out();
        }, this);
    }

    void enable(bool enabled) {
        _ticker.detach();
        Logger::info("ENABLE %s %s", name.c_str(), enabled ? "ON" : "OFF");
        pin_digital_write(_pin, enabled ? HIGH : LOW);
    }

    void dim(uint8_t value) {
        _ticker.detach();
        value *= 2;
        int analog_value = _dim_tab[value];
        Logger::info("DIM %s %d an %d", name.c_str(), value, analog_value);
        pin_analog_write(_pin, analog_value);
    }

    void set_random_midpoint(uint8_t percent) {
        _random_midpoint = RAND_MAX / (100.0 / _clamp((int)percent, MIN_RANDOM_MIDPOINT, MAX_RANDOM_MIDPOINT));
        Logger::info("RMID: %d%%", percent);
    }

    void set_heartbeat_max(uint8_t max) {
        _heartbeat_max = _clamp((int)max, FADE_MIN, FADE_MAX);
        Logger::info("HMAX: %d", _heartbeat_max);
    }

private:
    uint8_t         _pin;
    bool            _selected;
    int             _random_midpoint;
    int             _heartbeat_max;
    int             _heartbeat_delta = 1;
    TickerWrapper   _ticker;
    int             _fade_idx = FADE_MIN;
    static constexpr uint8_t _dim_tab[256] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  1,  1,  1, 1,  1,  1,  1,  1,  2,  2,  2,  2,  3,
        3,  3,  4,  4,  5,  5,  6,  6,  7,  7, 8,  9,  9, 10, 10, 11, 12, 13, 13, 14,
        15, 16, 17, 18, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 29, 30, 31, 32, 34, 35,
        36, 38, 39, 41, 42, 44, 45, 47, 49, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70,
        72, 74, 76, 78, 81, 83, 85, 88, 90, 93, 95, 98,101,103,106,109,111,114,117,120,
        123,126,129,132,135,138,141,145,148,151, 155,158,162,165,169,172,176,180,184,188,
        191,195,199,203,207,211,215,219,223,227, 231,235,239,243,247,251,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255, 255,255,255,255,255,255
    };

    void _toggle() {
        pin_digital_write(_pin, !pin_digital_read(_pin));
    }

    void _randomize() {
        bool enabled = rand() > _random_midpoint;
        pin_digital_write(_pin, enabled ? LOW : HIGH);
    }

    void _fade_in() {
        pin_analog_write(_pin, _dim_tab[_fade_idx++]);
        if (_fade_idx == FADE_MAX) {
            // fade finished
            _fade_idx = FADE_MIN;
            _ticker.detach();
        }
    }
    
    void _fade_out() {
        pin_analog_write(_pin, _dim_tab[_fade_idx--]);
        if (_fade_idx == FADE_MIN) {
            // fade finished
            _fade_idx = FADE_MAX;
            _ticker.detach();
        }
    }

    void _heartbeat(bool repeat) {
        _fade_idx += _heartbeat_delta;
        pin_analog_write(_pin, _dim_tab[_fade_idx]);
        if (_fade_idx == _heartbeat_max) {
            // fade in finished
            _heartbeat_delta = -1;
        }
        else if (_fade_idx == FADE_MIN) {
            // fade out finished
            _heartbeat_delta = 1;
            if (repeat == false) {
                // stop
                _ticker.detach();
            }
        }
    }
};
