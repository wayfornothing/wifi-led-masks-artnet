#pragma once

#include <map>
#include <string>

#include "hal/hal.h"
#include "led_device.h"

class DeviceConfig {
private:

    const String WIFI_CONFIG_FILE = "/wifi.json";
    const String LEDS_CONFIG_FILE = "/leds.json";

    // Singleton
    DeviceConfig() : _channel(1) {
        load();
    }
    DeviceConfig(const DeviceConfig&) = delete;
    DeviceConfig& operator=(const DeviceConfig&) = delete;

    // Configuration fields
    String _hostname;
    String _ssid;
    String _pass;
    uint16_t _channel;

    std::vector<LEDDevice> _leds;

public:
    static DeviceConfig& instance() {
        static DeviceConfig _instance;
        return _instance;
    }

    bool load() {
        if (!LittleFS.begin(true)) {
            // TODO: error mgmt
            Logger::error("WIFI/LEDS: LFS ERR");
            // TODO: error mgmt
            // return false;
            Logger::error("LittleFS failed to mount after format!");
            while (true) delay(1000);
        }

        // WIFI
        if (!LittleFS.exists(WIFI_CONFIG_FILE)) {
            // TODO: error mgmt
            Logger::error("WIFI: CFG ERR");
            return false;
        }
        File wifi_file = LittleFS.open(WIFI_CONFIG_FILE, "r");
        if (!wifi_file) {
            // TODO: error mgmt
            Logger::error("WIFI: OPEN ERR");
            return false;
        }
        DynamicJsonDocument wifi_json(2048);
        DeserializationError err = deserializeJson(wifi_json, wifi_file);
        wifi_file.close();
        if (err) {
            // TODO: error mgmt
            Logger::error("WIFI: JSON ERR");
            return false;
        }

        _hostname = wifi_json["hostname"] | "esp8266-device";
        _ssid     = wifi_json["ssid"]     | "";
        _pass     = wifi_json["pass"]     | "";

        // LEDS
        if (!LittleFS.exists(LEDS_CONFIG_FILE)) {
            // TODO: error mgmt
            Logger::error("LEDS: CFG ERR");
            return false;
        }
        File leds_file = LittleFS.open(LEDS_CONFIG_FILE, "r");
        if (!leds_file) {
            // TODO: error mgmt
            Logger::error("LEDS: OPEN ERR");
            return false;
        }
        DynamicJsonDocument leds_json(2048);
        err = deserializeJson(leds_json, leds_file);
        leds_file.close();
        if (err) {
            // TODO: error mgmt
            Logger::error("WIFI: JSON ERR");
            return false;
        }

        _channel  = leds_json["channel"] | 1;
        _leds.clear();
        if (leds_json.containsKey("leds")) {
            for (JsonObject led_json : leds_json["leds"].as<JsonArray>()) {
                // LEDConfig lc;
                auto name = led_json["name"];
                auto pin = pin_from_string(led_json["pin"]);
                if (pin != PIN_INVALID) {
                    LEDDevice led = LEDDevice(pin, name);
                    _leds.push_back(led);
                }
                else {
                    // TODO: err mgmt
                }
            }
        }
        
        Logger::info("LOAD: OK");
        debug();
        return true;
    }

    void debug() {
        Logger::info(_hostname.c_str());
        Logger::info(_ssid.c_str());
        Logger::info("CHAN: %d\n", _channel);
    }

    bool save_wifi() {
        if (!LittleFS.begin()) {
             // TODO: error mgmt
            Logger::error("WIFI: LFS ERR");
            return false;
        }

        DynamicJsonDocument doc(2048);
        doc["hostname"] = _hostname;
        doc["ssid"]     = _ssid;
        doc["pass"]     = _pass;

        File f = LittleFS.open(WIFI_CONFIG_FILE, "w");
        if (!f) {
            // TODO: error mgmt
            Logger::error("WIFI: LFS OPEN ERR");
            return false;
        }
        serializeJson(doc, f);
        f.close();
        Logger::info("WIFI: OK");
        debug();
        return true;
    }
        
    bool save_leds(String& raw_json) {
        bool ret = false;
        // String file = LEDS_CONFIG_FILE;
        // Logger::verbose(file.c_str());
        File f = LittleFS.open(LEDS_CONFIG_FILE, "w");
        if (f) {
            f.print(raw_json);
            f.close();
            Logger::info("LEDS SAVED");
            ret = true;
        }
        return ret;
    }
    
    // Accessors
    // TODO: meh ?
    const String& get_hostname() const { return _hostname; }
    void set_hostname(const String& h) { _hostname = h; }

    const String& get_SSID() const { return _ssid; }
    void set_SSID(const String& s) { _ssid = s; }

    const String& get_password() const { return _pass; }
    void set_password(const String& p) { _pass = p; }

    uint16_t get_channel() const { return _channel; }
    void set_channel(uint16_t c) { _channel = c; }

    std::vector<LEDDevice>& get_leds() { return _leds; }
};
