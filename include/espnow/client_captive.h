#pragma once

#include "hal/hal.h"
#include "espnow/client_config.h"
#include "client_captive.html.h"
#include "led_device.h"

#define AP_NAME "WFN-Config"

class CaptivePortal {

public:
    static void start_captive_portal() {
        

        Logger::info("MAC:%s ", WiFi.macAddress());
        
        const char* config_path = DeviceConfig::instance().get_config_path();


        LEDDevice _test_led(0, "test");

        LEDDevice led(LED_BUILTIN, "");
        led.blink(DEFAULT_BLINK_INTERVAL_MS);

        WebServer server(80);
        DNSServer dns;
        WiFi.mode(WIFI_AP);
        WiFi.softAP(AP_NAME);

        // captive portal redirect
        dns.start(53, "*", WiFi.softAPIP());
        server.onNotFound([&server]() {
            server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
            server.send(302, "text/plain", "");
        });

        server.on("/", HTTP_GET, [&server]() {
            String html = CLIENT_CAPTIVE_HTML;
            html.replace("__MAC_ADDRESS__", WiFi.macAddress());
            server.send(200, "text/html", html);
        });

        // save config
        server.on("/save", HTTP_POST, [&server]() {
            if (!server.hasArg("plain")) {
                server.send(400, "text/plain", "No body");
                Logger::error("save NO BODY");
                while(1);
            }

            String body = server.arg("plain");
            DeviceConfig::instance().save_config(body);
            server.send(200, "text/plain", "Saved");
            delay_ms(1000);
            reboot();
        });

        // test LED actions (unchanged from before)
        server.on("/test", HTTP_POST, [&server, &_test_led]() {
            if (!server.hasArg("plain")) {
                server.send(400, "text/plain", "No body");
                Logger::error("test NO BODY");
                while(1);
            }
            String body = server.arg("plain");
            StaticJsonDocument<512> doc;
            deserializeJson(doc, body);
            String mode = doc["mode"].as<String>();
            String pin_name = doc["pin"].as<String>();
            int pin = pin_from_string(pin_name);
            _test_led.change_pin(pin);

            if (mode == "enable") {
                static int en = 1;
                _test_led.enable(en);
                en = !en;
            } 
            else if (mode == "blink") {
                _test_led.blink(DEFAULT_BLINK_INTERVAL_MS);
            } 
            else if (mode == "random") {
                _test_led.random(DEFAULT_RANDOM_INTERVAL_MS);
            } 
            else if (mode == "fade") {
                _test_led.fade_in(DEFAULT_FADE_INTERVAL_MS);
            }
            else if (mode == "pulse") {
                _test_led.pulse(DEFAULT_FADE_INTERVAL_MS);
            }
            else if (mode == "heartbeat") {
                _test_led.heartbeat(DEFAULT_FADE_INTERVAL_MS);
            }
            server.send(200, "text/plain", "OK");
        });


        // check config file
        server.on(config_path, HTTP_GET, [&]() {
            if (fs_exists(config_path)) {
                auto f = fs_open(config_path, "r");
                server.streamFile(f, "application/json");
            }
            else {
                server.send(200, "text/plain", "config file missing");  // return empty array if file missing
            }
        });

        server.begin();
        Logger::info("Captive portal started at %s\nConnect to SSID: %s\n", WiFi.softAPIP().toString().c_str(), AP_NAME);

        // blocking on purpose
        while (1) {
            dns.processNextRequest();
            server.handleClient();
        }
    }
};