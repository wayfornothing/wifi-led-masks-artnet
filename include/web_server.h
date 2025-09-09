#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "device_config.h"

class ConfigWebServer {
public:
    ConfigWebServer(DeviceConfig& cfg)
        : _cfg(cfg), _server(80) {
            // Serial.begin(9600);
            Serial.println("ConfigWebServer CTOR OK");
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
    DeviceConfig& _cfg;
    ESP8266WebServer _server;

    void _handleRoot() {
        String html = "<html><body><h1>ESP Config</h1>";
        html += "<form method='POST' action='/save'>";

        html += "Hostname: <input name='hostname' value='" + _cfg.hostname + "'><br>";
        html += "Universe: <input name='universe' value='" + String(_cfg.universe) + "'><br>";
        html += "Channel: <input name='channel' value='" + String(_cfg.channel) + "'><br>";

        for (int i = 0; i < MAX_LEDS; i++) {
            html += "<h3>LED " + String(i+1) + "</h3>";
            html += "Pin name: <input name='led" + String(i) + "_name' value='" + _cfg.leds[i].name + "'><br>";
            html += "Description: <input name='led" + String(i) + "_desc' value='" + _cfg.leds[i].desc + "'><br>";
        }

        html += "<br><input type='submit' value='Save'></form></body></html>";
        _server.send(200, "text/html", html);
    }


    void _handleSave() {
        _cfg.hostname = _server.arg("hostname");
        _cfg.universe = _server.arg("universe").toInt();
        _cfg.channel  = _server.arg("channel").toInt();

        for (int i = 0; i < MAX_LEDS; i++) {
            _cfg.leds[i].name  = _server.arg("led" + String(i) + "_name");
            _cfg.leds[i].desc = _server.arg("led" + String(i) + "_desc");
            // _cfg.leds[i].desc = _server.arg("led" + String(i) + "_desc");
        }

        _cfg.save();
        _server.send(200, "text/html",
            "<html><body><h1>Saved, reboot device to apply!</h1><a href='/'>Back</a></body></html>");
    }
};
