#pragma once

#include <Arduino.h>
#include <Ticker.h>

class TickerWrapper {
public:
    TickerWrapper() {
    }

    ~TickerWrapper() {
        detach();
    }

    void attach_ms(uint32_t intervalMs, void (*callback)(void)) {
        _interval = intervalMs;
        _ticker.attach_ms(intervalMs, callback);
        _active = true;
    }

    // Instanciation explicite des templates pour quelques types courants
    // template void attach_ms<int>(uint32_t, void (*)(int), int);
    // template void attach_ms<void*>(uint32_t, void (*)(void*), void*);

    template <typename T>
    void attach_ms(uint32_t intervalMs, void (*callback)(T), T arg) {
        _interval = intervalMs;
        _ticker.attach_ms(intervalMs, callback, arg);
        _active = true;
    }
    
    void detach() {
        _ticker.detach();
        _active = false;
    }

    bool isActive() const {
        return _active;
    }

private:
    Ticker _ticker;
    uint32_t _interval = 0;
    bool _active = false;
};
