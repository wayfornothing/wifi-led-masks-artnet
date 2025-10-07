#pragma once 

#include <Arduino.h>

// Niveaux de log
enum LogLevel {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_VERBOSE
};

class Logger {
public:
    // Initialise avec un flux série
    static void begin_hw(unsigned long baud = 115200);

    // Configuration du niveau de log
    static void setLogLevel(LogLevel level);
    static LogLevel getLogLevel();

    // Active / désactive le logger globalement
    static void enable(bool state);
    static bool isEnabled();

    // Niveaux de log
    static void error(const char* format, ...);
    static void warn(const char* format, ...);
    static void info(const char* format, ...);
    static void debug(const char* format, ...);
    static void verbose(const char* format, ...);

private:
    static void logMessage(LogLevel level, const char* format, va_list args);
    static const char* levelToString(LogLevel level);
    static void printTimestamp();

    static HardwareSerial* _serial;
    static bool _enabled;
    static LogLevel _currentLevel;
};
