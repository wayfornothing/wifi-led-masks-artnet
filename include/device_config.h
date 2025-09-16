#pragma once
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <map>
#include <string>

class DeviceConfig {
public:    
    const String WIFI_CONFIG_FILE = "/wifi.json";
    const String LEDS_CONFIG_FILE = "/leds.json";
private:

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
            Serial.println(F("WIFI/LEDS: LFS ERR"));
            // TODO: error mgmt
            return false;
        }

        // WIFI
        if (!LittleFS.exists(WIFI_CONFIG_FILE)) {
            // TODO: error mgmt
            Serial.println(F("WIFI: CFG ERR"));
            return false;
        }
        File wifi_file = LittleFS.open(WIFI_CONFIG_FILE, "r");
        if (!wifi_file) {
            // TODO: error mgmt
            Serial.println(F("WIFI: OPEN ERR"));
            return false;
        }
        DynamicJsonDocument wifi_json(2048);
        DeserializationError err = deserializeJson(wifi_json, wifi_file);
        wifi_file.close();
        if (err) {
            // TODO: error mgmt
            Serial.println(F("WIFI: JSON ERR"));
            return false;
        }

        hostname = wifi_json["hostname"] | "esp8266-device";
        ssid     = wifi_json["ssid"]     | "";
        pass     = wifi_json["pass"]     | "";

        // LEDS
        if (!LittleFS.exists(LEDS_CONFIG_FILE)) {
            // TODO: error mgmt
            Serial.println(F("LEDS: CFG ERR"));
            return false;
        }
        File leds_file = LittleFS.open(LEDS_CONFIG_FILE, "r");
        if (!leds_file) {
            // TODO: error mgmt
            Serial.println(F("LEDS: OPEN ERR"));
            return false;
        }
        DynamicJsonDocument leds_json(2048);
        err = deserializeJson(leds_json, leds_file);
        leds_file.close();
        if (err) {
            // TODO: error mgmt
            Serial.println(F("WIFI: JSON ERR"));
            return false;
        }

        universe = leds_json["universe"]   | 0;
        channel  = leds_json["channel"]    | 1;

        leds.clear();
        if (leds_json.containsKey("leds")) {
            for (JsonObject led : leds_json["leds"].as<JsonArray>()) {
                LEDConfig lc;
                lc.name           = (const char*) led["name"];
                lc.desc           = (const char*) led["desc"];
                lc.pin            = led["pin"];
                lc.blink_ms  = led["blink_ms"]  | 0;
                lc.random_ms = led["random_ms"] | 0;
                lc.fade_ms   = led["fade_ms"]   | 0;
                leds.push_back(lc);
            }
        }
        
        Serial.println("LOAD: OK");
        debug();
        return true;
    }

    void debug() {
        Serial.println(hostname);
        Serial.println(ssid);
        // Serial.println(pass);
        Serial.printf("UNI: %d\n", universe);
        Serial.printf("CHAN: %d\n", channel);
    }

    bool save_wifi() {
        if (!LittleFS.begin()) {
             // TODO: error mgmt
            Serial.println(F("WIFI: LFS ERR"));
            return false;
        }

        DynamicJsonDocument doc(2048);
        doc["hostname"] = hostname;
        doc["ssid"]     = ssid;
        doc["pass"]     = pass;

        File f = LittleFS.open(WIFI_CONFIG_FILE, "w");
        if (!f) {
            // TODO: error mgmt
            Serial.println(F("WIFI: LFS OPEN ERR"));
            return false;
        }
        serializeJson(doc, f);
        f.close();
        Serial.println("WIFI: OK");
        debug();
        return true;
    }
        
    bool save_leds() {
        if (!LittleFS.begin()) {
             // TODO: error mgmt
            Serial.println("LEDS: LFS ERR");
            return false;
        }

        DynamicJsonDocument doc(2048);
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

        File f = LittleFS.open(LEDS_CONFIG_FILE, "w");
        if (!f) {
            // TODO: error mgmt
            Serial.println("SAVE: LFS OPEN ERR");
            return false;
        }
        serializeJson(doc, f);
        f.close();

        Serial.println("LEDS: OK");
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
