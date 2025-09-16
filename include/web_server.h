#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "index.html.h"
#include "device_config.h"

class ConfigWebServer {
public:
    ConfigWebServer()
        : _server(80) {
    }

    // void begin() {
    //     _server.on("/", HTTP_GET, [&]() { _handle_root(); });
    //     _server.on("/save", HTTP_POST, [&]() { _handle_save(); });
    //     _server.on("/test", HTTP_GET, [&]() { _handle_test(); });

    //     _server.begin();
    //     Serial.println("Web server started!");
    // }

    void tick() {
        _server.handleClient();
    }


    void _handle_test() {
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

        // check config.json if exists
        const auto LEDS_JSON = "/leds.json";
        _server.on(LEDS_JSON, HTTP_GET, [&]() {
            if (LittleFS.exists(LEDS_JSON)) {
                File f = LittleFS.open(LEDS_JSON, "r");
                _server.streamFile(f, "application/json");
                f.close();
                Serial.printf("%s LOAD OK\n", LEDS_JSON);
            } else {
                _server.send(200, "application/json", "{}");
                Serial.printf("%s NEW\n", LEDS_JSON);
            }
        });

        // save config.json
        _server.on("/save", HTTP_POST, [&]() {
            if (!_server.hasArg("plain")) {
                _server.send(400, "text/plain", "No body");
                Serial.println("save NO BODY");
                return;
            }

            String body = _server.arg("plain");
            File f = LittleFS.open(DeviceConfig::instance().LEDS_CONFIG_FILE, "w");
            f.print(body);
            f.close();

            Serial.println(body);
            Serial.println("SAVED");
            _server.send(200, "text/plain", "Saved");
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
            pinMode(pin, OUTPUT);

            if (mode == "enable") {
                //digitalWrite(pin, HIGH);
            } else if (mode == "blink") {
            // digitalWrite(pin, HIGH);
            // delay(200);
            // digitalWrite(pin, LOW);
            } else if (mode == "random") {
            // analogWrite(pin, random(0, 1024));
            } else if (mode == "fade") {
            // for (int i=0; i<=1023; i+=20) { analogWrite(pin, i); delay(10); }
            // for (int i=1023; i>=0; i-=20) { analogWrite(pin, i); delay(10); }
            }

            _server.send(200, "text/plain", "OK");
        });

        _server.begin();
    }

private:
    ESP8266WebServer _server;

    // void _handle_root() {
        
    //     String html = "<html><body><h1>ESP Config</h1>";
    //     html += "<form method='POST' action='/save'>";
        
    //     DeviceConfig& cfg = DeviceConfig::instance();
    //     html += "Hostname: <input name='hostname' value='" + cfg.get_hostname() + "'><br>";
    //     html += "Universe: <input name='universe' value='" + String(cfg.get_universe()) + "'><br>";
    //     html += "Channel: <input name='channel' value='" + String(cfg.get_channel()) + "'><br>";

    //     int i = 0;
    //     for (auto& led : cfg.get_leds()) {
    //         html += "<h3>LED " + String(i + 1) + "</h3>";
    //         html += "Pin name: <input name='led" + String(i) + "_name' value='" + led.name + "'><br>";
    //         html += "Description: <input name='led" + String(i) + "_desc' value='" + led.desc + "'><br>";
    //         i++;
    //     }

    //     html += "<br><input type='submit' value='Save'></form></body></html>";
    //     _server.send(200, "text/html", html);
    // }


    // void _handle_save() {
    //     DeviceConfig& cfg = DeviceConfig::instance();

    //     cfg.set_hostname(_server.arg("hostname"));
    //     cfg.set_universe(_server.arg("universe").toInt());
    //     cfg.set_channel(_server.arg("channel").toInt());

    //     int i = 0;
    //     for (auto& led : cfg.get_leds()) {
    //         led.name = _server.arg("led" + String(i) + "_name");
    //         led.desc = _server.arg("led" + String(i) + "_desc");
    //         i++;
    //     }

    //     cfg.save();
    //     _server.send(200, "text/html",
    //         "<html><body><h1>Saved, reboot device to apply!</h1><a href='/'>Back</a></body></html>");
    // }
};
