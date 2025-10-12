#pragma once 

#include <Arduino.h>
#include <stdarg.h>

enum LogLevel {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_VERBOSE
};

class Logger {
private:
    static inline HWCDC* _serial = nullptr;
    static inline bool _enabled = true;
    static inline LogLevel _currentLevel = LOG_LEVEL_DEBUG;

public:

    static void begin_hw(unsigned long baud = 115200) {
        _serial = &Serial;
        if (_serial) {
            _serial->begin(baud);
        }
    }

    static void setLogLevel(LogLevel level) {
        _currentLevel = level;
    }

    static LogLevel getLogLevel() {
        return _currentLevel;
    }

    static void enable(bool state) {
        _enabled = state;
    }

    static bool isEnabled() {
        return _enabled;
    }

    static void error(const char* format, ...) {
        va_list args;
        va_start(args, format);
        logMessage(LOG_LEVEL_ERROR, format, args);
        va_end(args);
    }

    static void warn(const char* format, ...) {
        va_list args;
        va_start(args, format);
        logMessage(LOG_LEVEL_WARN, format, args);
        va_end(args);
    }

    static void info(const char* format, ...) {
        va_list args;
        va_start(args, format);
        logMessage(LOG_LEVEL_INFO, format, args);
        va_end(args);
    }

    static void debug(const char* format, ...) {
        va_list args;
        va_start(args, format);
        logMessage(LOG_LEVEL_DEBUG, format, args);
        va_end(args);
    }

    static void verbose(const char* format, ...) {
        va_list args;
        va_start(args, format);
        logMessage(LOG_LEVEL_VERBOSE, format, args);
        va_end(args);
    }

private:

    static void printTimestamp() {
        if (!_serial) return;
        unsigned long now = millis();
        unsigned long seconds = now / 1000;
        unsigned long ms = now % 1000;

        _serial->printf("[%lu.%03lu] ", seconds, ms);
    }

    static void logMessage(LogLevel level, const char* format, va_list args) {
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

    static const char* levelToString(LogLevel level) {
        switch (level) {
            case LOG_LEVEL_ERROR: return "ERROR";
            case LOG_LEVEL_WARN:  return "WARN";
            case LOG_LEVEL_INFO:  return "INFO";
            case LOG_LEVEL_DEBUG: return "DEBUG";
            case LOG_LEVEL_VERBOSE: return "VERBOSE";
            default: return "NONE";
        }
    }
};


