#pragma once

#include <Arduino.h>
#include <Ticker.h>

typedef enum {
    LED_STATUS_IDLE,
    LED_STATUS_BLINK,
    LED_STATUS_FADE_IN,
    LED_STATUS_FADE_OUT,
    LED_STATUS_RANDOM
} eLEDStatus;   

#define FADE_MAX (512)

class LEDDevice {

public:
    uint32_t blink_interval_ms = 50;
    uint32_t random_interval_ms = 50;
    uint32_t fade_interval_ms = 2;
    String name;

    LEDDevice(uint8_t pin, String name) : 
        name(name),
        _pin(pin), 
        _status(LED_STATUS_IDLE) {
        pinMode(_pin, OUTPUT);
    }

    void start_blink() {
        Serial.println(F("BLINK START"));
        _status = LED_STATUS_BLINK;
        _ticker.detach();
        _ticker.attach_ms(blink_interval_ms, +[] (LEDDevice* self) {
            self->_toggle();
        }, this);
    }


    void start_random() {
        Serial.println(F("RANDOM START"));
        _status = LED_STATUS_RANDOM;
        _ticker.detach();
        _ticker.attach_ms(random_interval_ms, +[] (LEDDevice* self) {
            self->_randomize();
        }, this);
    }

    void start_fade_in() {
        Serial.println(F("FADE IN START"));
        enable(false);
        _status = LED_STATUS_FADE_IN;
        _duty = FADE_MAX;
        _ticker.attach_ms(fade_interval_ms, +[] (LEDDevice* self) {
            self->_fade_in();
        }, this);
    }

    void start_fade_out() {
        Serial.println(F("FADE OUT START"));
        enable(true);
        _status = LED_STATUS_FADE_OUT;
        _duty = 0;
        _ticker.attach_ms(fade_interval_ms, +[] (LEDDevice* self) {
            self->_fade_out();
        }, this);
    }

    void enable(bool enabled) {
        _ticker.detach();
        _status = LED_STATUS_IDLE;
        digitalWrite(_pin, enabled ? LOW : HIGH);
    }

    void stop_blink() {
        _ticker.detach();
    }
    
private:
    uint8_t _pin;
    Ticker _ticker;
    eLEDStatus _status;
    uint16_t _duty = 0;
    
    void _randomize() {
        bool enabled = rand() > (RAND_MAX / 2); // TODO: ratio should be adjustable
        digitalWrite(_pin, enabled ? LOW : HIGH);
    }

    void _toggle() {
        digitalWrite(_pin, !digitalRead(_pin));
    }

    void _fade_out() {
        // Serial.println(duty);
        analogWrite(_pin, _duty++);
        if (_duty == FADE_MAX) {
            // fade finished
            Serial.println(F("FADE OUT ENDED"));
            _duty = 0;
            _status = LED_STATUS_IDLE;
            _ticker.detach();
        }
    }
    
    void _fade_in() {
        // Serial.println(duty);
        analogWrite(_pin, _duty--);
        if (_duty == 0) {
            // fade finished
            Serial.println(F("FADE IN ENDED"));
            _duty = FADE_MAX;
            _status = LED_STATUS_IDLE;
            _ticker.detach();
        }
    }
};
