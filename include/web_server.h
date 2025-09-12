#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "device_config.h"

class ConfigWebServer {
public:
    ConfigWebServer()
        : _server(80) {
    }

    void begin() {
        _server.on("/", HTTP_GET, [&]() { _handleRoot(); });
        _server.on("/save", HTTP_POST, [&]() { _handleSave(); });
        _server.on("/test", HTTP_GET, [&]() { _handleTest(); });

        _server.begin();
        Serial.println("Web server started!");
    }

    void tick() {
        _server.handleClient();
    }


    void _handleTest() {
    }

private:
    // DeviceConfig& _cfg;
    ESP8266WebServer _server;

    void _handleRoot() {
        String html = "<html><body><h1>ESP Config</h1>";
        html += "<form method='POST' action='/save'>";

        DeviceConfig& cfg = DeviceConfig::instance();
        html += "Hostname: <input name='hostname' value='" + cfg.get_hostname() + "'><br>";
        html += "Universe: <input name='universe' value='" + String(cfg.get_universe()) + "'><br>";
        html += "Channel: <input name='channel' value='" + String(cfg.get_channel()) + "'><br>";

        int i = 0;
        for (auto& led : cfg.get_leds()) {
            html += "<h3>LED " + String(i + 1) + "</h3>";
            html += "Pin name: <input name='led" + String(i) + "_name' value='" + led.name + "'><br>";
            html += "Description: <input name='led" + String(i) + "_desc' value='" + led.desc + "'><br>";
            i++;
        }

        html += "<br><input type='submit' value='Save'></form></body></html>";
        _server.send(200, "text/html", html);
    }


    void _handleSave() {
        DeviceConfig& cfg = DeviceConfig::instance();

        cfg.set_hostname(_server.arg("hostname"));
        cfg.set_universe(_server.arg("universe").toInt());
        cfg.set_channel(_server.arg("channel").toInt());

        int i = 0;
        for (auto& led : cfg.get_leds()) {
            led.name = _server.arg("led" + String(i) + "_name");
            led.desc = _server.arg("led" + String(i) + "_desc");
            i++;
        }

        cfg.save();
        _server.send(200, "text/html",
            "<html><body><h1>Saved, reboot device to apply!</h1><a href='/'>Back</a></body></html>");
    }
};
