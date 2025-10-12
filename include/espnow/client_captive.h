#pragma once

#include "hal/hal.h"
#include "espnow/client_config.h"
#include "client.html.h"

class WiFiCaptivePortal {

    #define AP_NAME "WFN-Config"
    
    public:
    static void start_captive_portal() {
        
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
            String html = CLIENT_HTML;
            html.replace("__MAC_ADDRESS__", WiFi.macAddress());
            server.send(200, "text/html", CLIENT_HTML);
        });
        
        server.on("/save", HTTP_POST, [&server]() {
            const String channel = server.arg("channel");

            DeviceConfig& cfg = DeviceConfig::instance();
            cfg.set_channel(channel.toInt());
            // cfg.save_config();

            server.send(200, "text/html", "<html><body><h1>Saved. Rebooting...</h1></body></html>");
            delay_ms(1000);
            reboot();

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