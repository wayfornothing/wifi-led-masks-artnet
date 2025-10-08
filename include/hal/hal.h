#pragma once

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ArtnetWifi.h>
#include <ESP8266mDNS.h>

void reboot() {
    ESP.restart();
}

void wifi_set_hostname(const char * hostname) {
    WiFi.hostname(hostname);
}

void wifi_connect_to_ap(const char * ssid, const char * password) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
}

const char* wifi_get_local_ip() {
    static String ip = WiFi.localIP().toString();
    return ip.c_str();
}

int wifi_get_status() {
    return WiFi.status();
}

#define PIN_INVALID (0xff)
uint8_t pin_from_string(const String& pin_name) {
    if (pin_name == "D0") return D0;
    if (pin_name == "D1") return D1;
    if (pin_name == "D2") return D2;
    if (pin_name == "D3") return D3;
    if (pin_name == "D4") return D4;
    if (pin_name == "D5") return D5;
    if (pin_name == "D6") return D6;
    if (pin_name == "D7") return D7;
    if (pin_name == "D8") return D8;
    if (pin_name == "D9") return D9;
    if (pin_name == "D10") return D10;
    return PIN_INVALID;
}

#define WebServer ESP8266WebServer

void dns_update() {
    MDNS.update();
}
#endif



#ifdef ESP32

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ArtnetWifi.h>
#include <ESPmDNS.h>

void reboot() {
    ESP.restart();
}

void wifi_set_hostname(const char * hostname) {
    WiFi.hostname(hostname);
}

void wifi_connect_to_ap(const char * ssid, const char * password) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
}

const char* wifi_get_local_ip() {
    static String ip = WiFi.localIP().toString();
    return ip.c_str();
}

int wifi_get_status() {
    return WiFi.status();
}

#define PIN_INVALID (0xff)
uint8_t pin_from_string(const String& pin_name) {
    if (pin_name == "D0") return D0;
    if (pin_name == "D1") return D1;
    if (pin_name == "D2") return D2;
    if (pin_name == "D3") return D3;
    if (pin_name == "D4") return D4;
    if (pin_name == "D5") return D5;
    if (pin_name == "D6") return D6;
    if (pin_name == "D7") return D7;
    if (pin_name == "D8") return D8;
    // if (pin_name == "D9") return D9;
    // if (pin_name == "D10") return D10;
    return PIN_INVALID;
}

void dns_update() {
}

#endif

#ifdef ARDUINO

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "arduino/Logger.h"
#include "arduino/TickerWrapper.h"

// GPIO
inline void pin_set_output(int pin) {
    pinMode(pin, OUTPUT);
}

inline void pin_set_input(int pin) {
    pinMode(pin, INPUT);
}

inline void pin_digital_write(int pin, bool enable) {
    digitalWrite(pin, enable);
}

inline bool pin_digital_read(int pin) {
    return digitalRead(pin);
}

inline void pin_analog_write(int pin, int value) {
    analogWrite(pin, value);
}

void delay_ms(unsigned long ms) {
    delay(ms);
}

#endif

