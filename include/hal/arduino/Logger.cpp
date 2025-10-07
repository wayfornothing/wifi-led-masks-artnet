#include "Logger.h"
#include <stdarg.h>

HardwareSerial* Logger::_serial = nullptr;
bool Logger::_enabled = true;
LogLevel Logger::_currentLevel = LOG_LEVEL_DEBUG; // Par défaut

void Logger::begin_hw(unsigned long baud) {
    _serial = &Serial;
    if (_serial) {
        _serial->begin(baud);
    }
}

void Logger::setLogLevel(LogLevel level) {
    _currentLevel = level;
}

LogLevel Logger::getLogLevel() {
    return _currentLevel;
}

void Logger::enable(bool state) {
    _enabled = state;
}

bool Logger::isEnabled() {
    return _enabled;
}

void Logger::printTimestamp() {
    if (!_serial) return;
    unsigned long now = millis();
    unsigned long seconds = now / 1000;
    unsigned long ms = now % 1000;

    _serial->printf("[%lu.%03lu] ", seconds, ms);
}

void Logger::logMessage(LogLevel level, const char* format, va_list args) {
    if (!_enabled || !_serial) return;
    if (level > _currentLevel) return;

    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);

    // [time] [LEVEL] message
    printTimestamp();
    _serial->print("[");
    _serial->print(levelToString(level));
    _serial->print("] ");
    _serial->println(buffer);
}

const char* Logger::levelToString(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_WARN:  return "WARN";
        case LOG_LEVEL_INFO:  return "INFO";
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_VERBOSE: return "VERBOSE";
        default: return "NONE";
    }
}

// Fonctions publiques
void Logger::error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logMessage(LOG_LEVEL_ERROR, format, args);
    va_end(args);
}

void Logger::warn(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logMessage(LOG_LEVEL_WARN, format, args);
    va_end(args);
}

void Logger::info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logMessage(LOG_LEVEL_INFO, format, args);
    va_end(args);
}

void Logger::debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logMessage(LOG_LEVEL_DEBUG, format, args);
    va_end(args);
}

void Logger::verbose(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logMessage(LOG_LEVEL_VERBOSE, format, args);
    va_end(args);
}
