#pragma once

#include <map>
#include <string>

#include "hal/hal.h"
#include "led_device.h"

class DeviceConfig {
private:
    // Singleton
    DeviceConfig() : _channel(1) {
        load();
    }
    DeviceConfig(const DeviceConfig&) = delete;
    DeviceConfig& operator=(const DeviceConfig&) = delete;

    // Configuration fields
    String _hostname;
    uint8_t _channel;
    std::vector<LEDDevice> _leds;

    static constexpr const char* CONFIG_FILE = "/config.json";
public:
    static DeviceConfig& instance() {
        static DeviceConfig _instance;
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
            File leds_file = fs_open(CONFIG_FILE, "w");
            if (leds_file) {
                leds_file.print("{}"); // empty json
                leds_file.close();
            }
            else {
                // TODO: error mgmt
                Logger::error("CONFIG: DEFAULT OPEN ERR");
                return false;
            }
        }
        File leds_file = fs_open(CONFIG_FILE, "r");
        if (!leds_file) {
            // TODO: error mgmt
            Logger::error("CONFIG: OPEN ERR");
            return false;
        }
        DynamicJsonDocument leds_json(2048);
        DeserializationError err = deserializeJson(leds_json, leds_file);
        leds_file.close();
        if (err) {
            // TODO: error mgmt
            Logger::error("CONFIG: JSON ERR");
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
    
    // Accessors
    // TODO: meh ?
    const String& get_hostname() const { return _hostname; }
    void set_hostname(const String& h) { _hostname = h; }

    uint8_t get_channel() const { return _channel; }
    void set_channel(uint8_t c) { _channel = c; }

    std::vector<LEDDevice>& get_leds() { return _leds; }
    const char* get_config_path() { return CONFIG_FILE; }
};
