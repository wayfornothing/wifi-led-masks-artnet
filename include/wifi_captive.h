#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "device_config.h"
#include "captive.html.h"

class WiFiCaptivePortal {

    #define AP_NAME "WFN-Config"
    
    public:
    static void start_captive_portal() {
        
        LEDDevice led(LED_BUILTIN, "");
        led.start_blink(DEFAULT_BLINK_INTERVAL_MS);

        ESP8266WebServer server(80);
        DNSServer dns;
        WiFi.mode(WIFI_AP);
        WiFi.softAP(AP_NAME);
        dns.start(53, "*", WiFi.softAPIP());

        // captive portal redirect
        server.onNotFound([&server]() {
            server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
            server.send(302, "text/plain", "");
        });

        server.on("/", HTTP_GET, [&server]() {
            // String page = "<html><body><h1>WiFi Config</h1>"
            //               "<form method='POST' action='/save'>"
            //               "SSID: <input name='ssid'><br>"
            //               "Password: <input name='password' type='password'><br>"
            //               "Device name: <input name='hostname'><br>"
            //               "<input type='submit' value='Save'>"
            //               "</form></body></html>";
            server.send(200, "text/html", CAPTIVE_HTML);

        });
        server.on("/save", HTTP_POST, [&server]() {
            const String ssid = server.arg("ssid");
            const String pass = server.arg("password");
            const String name = server.arg("hostname");

            DeviceConfig& cfg = DeviceConfig::instance();
            cfg.set_SSID(ssid);
            cfg.set_password(pass);
            cfg.set_hostname(name);
            cfg.save_wifi();

            server.send(200, "text/html", "<html><body><h1>Saved. Rebooting...</h1></body></html>");
            delay(1000);
            ESP.restart();

        });
        server.begin();

        Serial.printf("Captive portal started at %s\nConnect to SSID: %s\n", WiFi.softAPIP().toString().c_str(), AP_NAME);

        // blocking
        while (1) {
            dns.processNextRequest();
            server.handleClient();
        }
    }
};