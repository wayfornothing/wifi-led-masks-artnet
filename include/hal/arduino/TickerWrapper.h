#pragma once

#include <Arduino.h>
#include <Ticker.h>

class TickerWrapper {
public:
    TickerWrapper();
    ~TickerWrapper();

    // Attache une fonction sans argument
    void attach_ms(uint32_t intervalMs, void (*callback)(void));

    // Attache une fonction avec argument (template)
    template <typename T>
    void attach_ms(uint32_t intervalMs, void (*callback)(T), T arg);

    // Détache le ticker
    void detach();

    // Redémarre le ticker avec le même intervalle
    void restart();

    // Vérifie si le ticker est actif
    bool isActive() const;

private:
    Ticker _ticker;
    uint32_t _interval = 0;
    bool _active = false;
};
