#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "index.html.h"
#include "led_device.h"
#include "device_config.h"

class ConfigWebServer {
public:
    ConfigWebServer()
        : _server(80) {
    }

    void tick() {
        _server.handleClient();
    }

    uint8_t _pin_from_string(const String& pin_name) {
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
        return 0xFF;
    }

    void begin() {
        // main page
        _server.on("/", HTTP_GET, [&]() {
            _server.send_P(200, "text/html", INDEX_HTML);

        });


        // save leds.json
        _server.on("/save", HTTP_POST, [&]() {
            if (!_server.hasArg("plain")) {
                _server.send(400, "text/plain", "No body");
                Serial.println("save NO BODY");
                return;
            }

            String body = _server.arg("plain");
            String file = DeviceConfig::instance().LEDS_CONFIG_FILE;
            Serial.print(file);
            File f = LittleFS.open(file, "w");
            f.print(body);
            f.close();

            Serial.println(body);
            Serial.println("LEDS SAVED");
            _server.send(200, "text/plain", "Saved");
        });



        _server.on("/leds.json", HTTP_GET, [&]() {
            if (LittleFS.exists("/leds.json")) {
                File f = LittleFS.open("/leds.json", "r");
                if (f) {
                    _server.streamFile(f, "application/json");
                    f.close();
                } else {
                    _server.send(500, "text/plain", "Failed to open /leds.json");
                }
            } else {
                _server.send(200, "text/plain", "/leds.json file missing");  // return empty array if file missing
            }

        });

        _server.on("/wifi.json", HTTP_GET, [&]() {
            if (LittleFS.exists("/wifi.json")) {
                File f = LittleFS.open("/wifi.json", "r");
                if (f) {
                    _server.streamFile(f, "application/json");
                    f.close();
                } else {
                    _server.send(500, "text/plain", "Failed to open /wifi.json");
                }
            } else {
                _server.send(200, "text/plain", "/wifi.json file missing");  // return empty array if file missing
            }
        });

        // test LED actions (unchanged from before)
        _server.on("/test", HTTP_POST, [&]() {
            if (!_server.hasArg("plain")) {
                _server.send(400, "text/plain", "No body");
                Serial.println("test NO BODY");
                return;
            }
            String body = _server.arg("plain");
            StaticJsonDocument<512> doc;
            deserializeJson(doc, body);
            String mode = doc["mode"].as<String>();
            String pinStr = doc["pin"].as<String>();
            int pin = _pin_from_string(pinStr);
            
            if (mode == "enable") {
                static int en = 1;
                LEDDevice led(pin, "test");
                led.enable(en != 0);
                en = !en;
                // pinMode(pin, OUTPUT);
                // digitalWrite(pin, !en);
            } 
            else if (mode == "blink") {
                LEDDevice led(pin, "test");
                led.start_blink();

            // digitalWrite(pin, HIGH);
            // delay(200);
            // digitalWrite(pin, LOW);
            } 
            else if (mode == "random") {
                LEDDevice led(pin, "test");
                led.start_random();
            // // analogWrite(pin, random(0, 1024));
            } else if (mode == "fade") {
                LEDDevice led(pin, "test");
                led.start_fade_in();
            // for (int i=0; i<=1023; i+=20) { analogWrite(pin, i); delay(10); }
            // for (int i=1023; i>=0; i-=20) { analogWrite(pin, i); delay(10); }
            }

            Serial.println("--TEST--");
            Serial.println(mode);
            Serial.println(pinStr);
            Serial.println(pin);
            Serial.println();


            _server.send(200, "text/plain", "OK");
        });

        _server.begin();
    }

private:
    ESP8266WebServer _server;
};
