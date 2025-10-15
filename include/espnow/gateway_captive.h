#pragma once

#include "hal/hal.h"
#include "led_device.h"
#include "web/gateway_captive.html.h"

#include <functional>

#define MAX_CLIENTS 10
#define MAC_ADDR_DATA_SZ (6)

typedef struct {
    const char* name;
    uint8_t channel;
    uint8_t mac_address[MAC_ADDR_DATA_SZ];
    uint32_t seq;
} client_data_t;


class GatewayConfig {
private:
    GatewayConfig() {
        Logger::begin_hw(115200);
        load();
    }
    GatewayConfig(const GatewayConfig&) = delete;
    GatewayConfig& operator=(const GatewayConfig&) = delete;

    std::vector<client_data_t> _clients;
    static constexpr const char* CONFIG_FILE = "/config.json";
public:
    static GatewayConfig& instance() {
        static GatewayConfig _instance;
        return _instance;
    }

    bool load() {
        if (!fs_begin()) {
            // TODO: error mgmt
            Logger::error("CONFIG: LFS ERR");
            // TODO: error mgmt
            // return false;
            Logger::error("File system failed to mount after format!");
            while (true) delay(1000);
        }

        if (!fs_exists(CONFIG_FILE)) {
            Logger::warn("CONFIG: no config, creating default...");
            File f = fs_open(CONFIG_FILE, "w");
            if (f) {
                f.print("{}"); // empty json
                f.close();
            }
            else {
                // TODO: error mgmt
                Logger::error("CONFIG: DEFAULT OPEN ERR");
                return false;
            }
        }

        File f = fs_open(CONFIG_FILE, "r");
        if (!f) {
            // TODO: error mgmt
            Logger::error("CONFIG: OPEN ERR");
            return false;
        }
        DynamicJsonDocument config_json(2048);
        DeserializationError err = deserializeJson(config_json, f);
        f.close();

        if (err) {
            // TODO: error mgmt
            Logger::error("CONFIG: JSON ERR");
            return false;
        }

        if (config_json.containsKey("clients")) {
            for (JsonObject client_json : config_json["clients"].as<JsonArray>()) {
                client_data_t client = { .name = client_json["name"],
                                         .channel = client_json["channel"],
                                         .seq = 0 };
                // parse mac addr
                sscanf(client_json["mac"], "%x:%x:%x:%x:%x:%x", &(client.mac_address[0]), &(client.mac_address[1]), 
                                                                &(client.mac_address[2]), &(client.mac_address[3]),
                                                                &(client.mac_address[4]), &(client.mac_address[5]));
                
                Logger::info("Loaded client %s channel #%d mac %x:%x:%x:%x:%x:%x", 
                              client.name, client.channel,
                              client.mac_address[0], client.mac_address[1], client.mac_address[2], 
                              client.mac_address[3], client.mac_address[4], client.mac_address[5]);
                _clients.push_back(client);
            }
        }
        
        Logger::info("%s LOAD: OK", CONFIG_FILE);
        return true;
    }

    bool save_config(String& raw_json) {
        bool ret = false;
        File f = fs_open(CONFIG_FILE, "w");
        if (f) {
            f.print(raw_json);
            f.close();
            Logger::info("CONFIG SAVED");
            ret = true;
        }
        return ret;
    }
    
    std::vector<client_data_t>& get_clients() { return _clients; }
    auto get_config_path() { return CONFIG_FILE; }
}; 


class CaptivePortal {

#define AP_NAME "WFN-Gateway"

public:
    static void start_captive_portal() {

        LEDDevice led(LED_BUILTIN, "");
        led.blink(DEFAULT_BLINK_INTERVAL_MS);

        std::vector<client_data_t>& clients = GatewayConfig::instance().get_clients();
        auto config_path = GatewayConfig::instance().get_config_path();

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
            server.send(200, "text/html", GATEWAY_CAPTIVE_HTML);
        });

        // List registered clients
        server.on(config_path, HTTP_GET, [&]() {
            if (fs_exists(config_path)) {
                auto f = fs_open(config_path, "r");
                server.streamFile(f, "application/json");
            }
            else {
                server.send(200, "text/plain", "config file missing");
            }
        });

        server.on("/save", HTTP_POST, [&server]() {
            if (!server.hasArg("plain")) {
                server.send(400, "text/plain", "No body");
                Logger::error("save NO BODY");
                while(1);
            }

            String body = server.arg("plain");
            GatewayConfig::instance().save_config(body);
            server.send(200, "text/plain", "Saved");
            delay_ms(1000);
            reboot();
        });

        Logger::info("Captive portal started at %s\n", WiFi.softAPIP().toString().c_str());

        server.begin();
        
        // blocking on purpose
        while (1) {
            dns.processNextRequest();
            server.handleClient();
        }
    }
};