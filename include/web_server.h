#pragma once

#include "hal/hal.h"
#include "index.html.h"
#include "led_device.h"
#include "device_config.h"

class ConfigWebServer {
private:
    WebServer _server;

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
                Logger::error("save NO BODY");
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
                Logger::error("test NO BODY");
                return;
            }
            String body = _server.arg("plain");
            StaticJsonDocument<512> doc;
            deserializeJson(doc, body);
            String mode = doc["mode"].as<String>();
            String pin_name = doc["pin"].as<String>();
            int pin = pin_from_string(pin_name);

            if (mode == "enable") {
                static int en = 1;
                LEDDevice led(pin, "test");
                led.enable(en);
                en = !en;
            } 
            else if (mode == "blink") {
                LEDDevice led(pin, "test");
                led.blink(DEFAULT_BLINK_INTERVAL_MS);
            } 
            else if (mode == "random") {
                LEDDevice led(pin, "test");
                led.random(DEFAULT_RANDOM_INTERVAL_MS);
            } else if (mode == "fade") {
                LEDDevice led(pin, "test");
                led.fade_in(DEFAULT_FADE_INTERVAL_MS);
            }
            _server.send(200, "text/plain", "OK");
        });

        _server.begin();
    }
};
