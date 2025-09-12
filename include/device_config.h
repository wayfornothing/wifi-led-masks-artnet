#pragma once
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <map>
#include <string>

class DeviceConfig {
private:

    const String CONFIG_FILE = "/config.json";

    // Singleton
    DeviceConfig() : universe(0), channel(1) {
        load();
    }
    DeviceConfig(const DeviceConfig&) = delete;
    DeviceConfig& operator=(const DeviceConfig&) = delete;

    // Configuration fields
    String hostname;
    String ssid;
    String pass;
    uint16_t universe;
    uint16_t channel;

    struct LEDConfig {
        String name;
        String desc;
        uint8_t pin;
        uint16_t blink_ms;
        uint16_t random_ms;
        uint16_t fade_ms;
    };

    std::vector<LEDConfig> leds;

public:
    static DeviceConfig& instance() {
        static DeviceConfig _instance;
        return _instance;
    }

    bool load() {
        if (!LittleFS.begin()) {
            // TODO: error mgmt
            Serial.println("LOAD: LFS ERR");
            return false;
        }
        if (!LittleFS.exists(CONFIG_FILE)) {
            Serial.println("LOAD: CFG ERR");
            return false;
        }

        File f = LittleFS.open(CONFIG_FILE, "r");
        if (!f) {
            Serial.println("LOAD: OPEN ERR");
            return false;
        }

        DynamicJsonDocument doc(2048);
        DeserializationError err = deserializeJson(doc, f);
        f.close();
        if (err) {
            // TODO: error mgmt
            Serial.println("LOAD: JSON ERR");
            return false;
        }

        hostname    = doc["hostname"]   | "esp8266-device";
        ssid        = doc["ssid"]       | "";
        pass        = doc["pass"]       | "";
        universe    = doc["universe"]   | 0;
        channel     = doc["channel"]    | 1;

        leds.clear();
        if (doc.containsKey("leds")) {
            for (JsonObject led : doc["leds"].as<JsonArray>()) {
                LEDConfig cfg;
                cfg.name           = (const char*) led["name"];
                cfg.desc           = (const char*) led["desc"];
                cfg.pin            = led["pin"];
                cfg.blink_ms  = led["blink_ms"]  | 0;
                cfg.random_ms = led["random_ms"] | 0;
                cfg.fade_ms   = led["fade_ms"]   | 0;
                leds.push_back(cfg);
            }
        }
        
        Serial.println("LOAD: OK");
        debug();
        return true;
    }


    void debug() {
        Serial.println(hostname);
        Serial.println(ssid);
        Serial.println(universe);
        Serial.println(channel);
    }


    bool save() {
        if (!LittleFS.begin()) {
             // TODO: error mgmt
            Serial.println("SAVE: LFS ERR");
            return false;
        }

        DynamicJsonDocument doc(2048);
        doc["hostname"] = hostname;
        doc["ssid"]     = ssid;
        doc["pass"]     = pass;
        doc["universe"] = universe;
        doc["channel"]  = channel;

        JsonArray ledsArr = doc.createNestedArray("leds");
        for (auto& l : leds) {
            JsonObject led = ledsArr.createNestedObject();
            led["name"]         = l.name;
            led["desc"]         = l.desc;
            led["pin"]          = l.pin;
            led["blink_ms"]     = l.blink_ms;
            led["random_ms"]    = l.random_ms;
            led["fade_ms"]      = l.fade_ms;
        }

        File f = LittleFS.open(CONFIG_FILE, "w");
        if (!f) {
            // TODO: error mgmt
            Serial.println("SAVE: LFS OPEN ERR");
            return false;
        }
        serializeJson(doc, f);
        f.close();

        Serial.println("SAVE: OK");
debug();
        return true;
    }

    // Accessors
    const String& get_hostname() const { return hostname; }
    void set_hostname(const String& h) { hostname = h; }

    const String& get_SSID() const { return ssid; }
    void set_SSID(const String& s) { ssid = s; }

    const String& get_password() const { return pass; }
    void set_password(const String& p) { pass = p; }

    uint16_t get_universe() const { return universe; }
    void set_universe(uint16_t u) { universe = u; }

    uint16_t get_channel() const { return channel; }
    void set_channel(uint16_t c) { channel = c; }

    std::vector<LEDConfig>& get_leds() { return leds; }
};
