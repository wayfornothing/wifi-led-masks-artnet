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

            DeviceConfig::instance().save_leds(body);

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
            String pin_name = doc["pin"].as<String>();
            int pin = DeviceConfig::pin_from_string(pin_name);

            Serial.println("--TEST--");
            Serial.println(mode);
            Serial.println(pin_name);
            Serial.println(pin);

            if (mode == "enable") {
                LEDDevice led(pin, "test");
                led.toggle();
            } 
            else if (mode == "blink") {
                LEDDevice led(pin, "test");
                led.start_blink();
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
            _server.send(200, "text/plain", "OK");
        });

        _server.begin();
    }

private:
    ESP8266WebServer _server;
};
