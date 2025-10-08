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

#define WebServer ESP8266WebServer
#endif



#ifdef ESP32

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ArtnetWifi.h>
#include <mDNS.h>

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

