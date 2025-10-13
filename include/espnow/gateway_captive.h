#pragma once

#include "hal/hal.h"
#include "device_config.h"
#include "gateway.html.h"
#include "mac_loader.h"

#include <functional>

// std::vector<String> _macs;

class CaptivePortal {

#define AP_NAME "WFN-Config"

public:
    static void start_captive_portal() {

        LEDDevice led(LED_BUILTIN, "");
        led.blink(DEFAULT_BLINK_INTERVAL_MS);

        display_print_str(AP_NAME, 15, 25);
        std::vector<String> _macs = MACLoader::load_mac_adresses();

        AsyncWebServer server(80);
        DNSServer dns;
        WiFi.mode(WIFI_AP);
        WiFi.softAP(AP_NAME);
        dns.start(53, "*", WiFi.softAPIP());

        // captive portal redirect
        // server.onNotFound([&server]()
        //                   {
        //     server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
        //     server.send(302, "text/plain", ""); });

        server.onNotFound([](AsyncWebServerRequest *request) {
                // request.se
            request->send(302, "text/plain", "");
        });

        // List registered clients
        server.on("/list", HTTP_GET, [&_macs](AsyncWebServerRequest *request) {
            DynamicJsonDocument doc(1024);
            JsonArray arr = doc.to<JsonArray>();
            for (auto &mac : _macs) {
                arr.add(mac);
            }
            String json;
            serializeJson(arr, json);
            request->send(200, "application/json", json); 
        });

        // Register a new client
        server.on("/register", HTTP_GET, [&_macs](AsyncWebServerRequest *request) {
            if (request->hasParam("mac")) {
                String mac = request->getParam("mac")->value();
                mac.toUpperCase();
                if (std::find(_macs.begin(), _macs.end(), mac) == _macs.end()) {
                    _macs.push_back(mac);
                    Serial.printf("Registered new client: %s\n", mac.c_str());
                }
                request->send(200, "text/plain", "OK");
            } else {
                request->send(400, "text/plain", "Missing 'mac' param");
            } 
        });

        Logger::info("Captive portal started at %s\n", WiFi.softAPIP().toString().c_str());

        server.begin();
        // blocking on purpose
        // while (1)
        // {
        //     dns.processNextRequest();
        //     server.handleClient();
        // }
    }
};