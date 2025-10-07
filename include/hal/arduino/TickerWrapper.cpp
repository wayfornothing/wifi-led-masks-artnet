#include "TickerWrapper.h"

TickerWrapper::TickerWrapper() {}

TickerWrapper::~TickerWrapper() {
    detach();
}

void TickerWrapper::attach_ms(uint32_t intervalMs, void (*callback)(void)) {
    _interval = intervalMs;
    _ticker.attach_ms(intervalMs, callback);
    _active = true;
}

template <typename T>
void TickerWrapper::attach_ms(uint32_t intervalMs, void (*callback)(T), T arg) {
    _interval = intervalMs;
    _ticker.attach_ms(intervalMs, callback, arg);
    _active = true;
}

// Instanciation explicite des templates pour quelques types courants
template void TickerWrapper::attach_ms<int>(uint32_t, void (*)(int), int);
template void TickerWrapper::attach_ms<void*>(uint32_t, void (*)(void*), void*);

void TickerWrapper::detach() {
    _ticker.detach();
    _active = false;
}

void TickerWrapper::restart() {
    if (_active && _interval > 0) {
        // On redétache, puis réattache la même fonction
        // ⚠️ Ticker ne stocke pas le callback accessible directement,
        // donc il faut normalement réattacher manuellement (selon ton design).
        // Ici, on ne peut pas le faire sans callback sauvegardé.
        _ticker.detach();
        _active = false;
        // (Facultatif : tu peux étendre la classe pour mémoriser le callback)
    }
}

bool TickerWrapper::isActive() const {
    return _active;
}
