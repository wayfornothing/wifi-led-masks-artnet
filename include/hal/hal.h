#pragma once

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

bool fs_exists(const char* path) {
    return LittleFS.exists(path);
}

File fs_open(const char* path, const char* mode) {
    return LittleFS.open(path, mode);
}

#endif


#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ArtnetWifi.h>
#include <ESP8266mDNS.h>
#include <espnow.h>

#define ESP_OK (0)

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

#define PIN_RESET_BUTTON (D0) // add a 10k resistor from D0 to 3v3R
void config_reset_button() {
    pin_set_input(PIN_RESET_BUTTON);
}

bool is_reset_button_pressed() {
    return pin_digital_read(PIN_RESET_BUTTON) == LOW;
}

bool fs_begin() {
    return LittleFS.begin();
}


#endif


#ifdef ESP32

#include <U8g2lib.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ArtnetWifi.h>
#include <ESPmDNS.h>
#include <esp_now.h>
// #include <ESPAsyncWebServer.h>


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
    if (pin_name == "D0") return 0;
    if (pin_name == "D1") return 1;
    if (pin_name == "D2") return 2;
    if (pin_name == "D3") return 3;
    if (pin_name == "D4") return 4;
    if (pin_name == "D5") return 5;
    if (pin_name == "D6") return 6;
    if (pin_name == "D7") return 7;
    if (pin_name == "D8") return 8;
    // if (pin_name == "D9") return D9;
    // if (pin_name == "D10") return D10;
    return PIN_INVALID;
}

void dns_update() {
}

#define PIN_RESET_BUTTON (0)
void config_reset_button() {
    pin_set_input(PIN_RESET_BUTTON);
}

bool is_reset_button_pressed() {
    return pin_digital_read(PIN_RESET_BUTTON) == HIGH;
}

bool fs_begin() {
    return LittleFS.begin(true);
}

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6, 5);

void display_init() {
    u8g2.begin();
    u8g2.setContrast(255);    // set contrast to maximum
    u8g2.setBusClock(400000); // 400kHz I2C
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.clearBuffer();

}


void display_print_str(const char* str, int x, int y) {
    static const int xOffset = 30; // = (132-w)/2
    static const int yOffset = 12; // = (64-h)/2

    u8g2.clearBuffer(); // clear the internal memory
    u8g2.setCursor(xOffset + x, yOffset + y);
    u8g2.printf(str);
    u8g2.sendBuffer(); // transfer internal memory to the display
}
#endif
