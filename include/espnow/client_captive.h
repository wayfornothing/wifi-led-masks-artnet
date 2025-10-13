#pragma once

#include "hal/hal.h"
#include "espnow/client_config.h"
#include "index.html.h"
#include "led_device.h"


class CaptivePortal {

    #define AP_NAME "WFN-Config"
    
public:
    static void start_captive_portal() {

        Logger::info("MAC:%s ", WiFi.macAddress());
        
        LEDDevice led(LED_BUILTIN, "");
        led.blink(DEFAULT_BLINK_INTERVAL_MS);

        WebServer server(80);
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
            String html = INDEX_HTML;
            html.replace("__MAC_ADDRESS__", WiFi.macAddress());
            server.send(200, "text/html", html);
        });
        
        // server.on("/save", HTTP_POST, [&server]() {
        //     const String channel = server.arg("channel");

        //     DeviceConfig& cfg = DeviceConfig::instance();
        //     cfg.set_channel(channel.toInt());
        //     // cfg.save_config();

        //     server.send(200, "text/html", "<html><body><h1>Saved. Rebooting...</h1></body></html>");
        //     delay_ms(1000);
        //     reboot();
        // });



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
        server.on("/test", HTTP_POST, [&server]() {
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
            } 
            else if (mode == "fade") {
                LEDDevice led(pin, "test");
                led.fade_in(DEFAULT_FADE_INTERVAL_MS);
            }
            server.send(200, "text/plain", "OK");
        });


        // check config file
        server.on(DeviceConfig::CONFIG_FILE, HTTP_GET, [&]() {
            if (LittleFS.exists(DeviceConfig::CONFIG_FILE)) {
                File f = LittleFS.open(DeviceConfig::CONFIG_FILE, "r");
                if (f) {
                    server.streamFile(f, "application/json");
                    f.close();
                } else {
                    server.send(500, "text/plain", "Failed to open config file");
                }
            } else {
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