#pragma once

#ifdef ARDUINO

#include "arduino/Logger.h"
#include "arduino/TickerWrapper.h"

// GPIO
inline void pin_set_output(int pin) {
    pinMode(pin, OUTPUT);
}

inline void pin_set_input(int pin) {
    pinMode(pin, INPUT);
}

inline void pin_write_digital(int pin, bool enable) {
    digitalWrite(pin, enable);
}

inline bool pin_read_digital(int pin) {
    return digitalRead(pin);
}

inline void pin_write_analog(int pin, int value) {
    analogWrite(pin, value);
}

void delay_ms(unsigned long ms) {
    delay(ms);
}

#endif