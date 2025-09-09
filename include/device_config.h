#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <map>

#define MAX_LEDS (4)

class DeviceConfig {

private:
    struct sLedConfig {
        String name;    // "D2", "D5", etc
        String desc;    // long description
        uint8_t pin;    // real pin number
    };

    const char* CONFIG_FILE = "/config.json";

    
    const std::map<std::string, uint8_t> _pin_map {
        { "D0", D0 }, { "D1", D1 }, { "D2", D2 }, { "D3", D3 },
        { "D4", D4 }, { "D5", D5 }, { "D6", D6 }, { "D7", D7 },
        { "D8", D8 }, { "D9", D9 }, { "D10", D10 }
    };

    uint8_t _pin_from_string(String pin_name) {
        auto it = _pin_map.find(std::string(pin_name.c_str()));
        if (it != _pin_map.end()) {
            // Serial.printf("MAP %s %d\n", pin_name.c_str(), it->second);
            return it->second;
        }
        // Serial.printf("MAP %s not found\n", pin_name.c_str());
        return 0xFF; // invalid pin
    }

public:
    String hostname;
    int universe;
    int channel;
    sLedConfig leds[MAX_LEDS];
    // uint8_t pins[MAX_LEDS];

    // TODO:
    // String ssid;
    // String pass;
    // uint32_t    connect_timeout_ms;
    // uint32_t    connect_retry_ms;

public:
    DeviceConfig() {
        Serial.begin(9600);
        // Serial.println("DeviceConfig CTOR");

        if (!LittleFS.begin()) {
            Serial.println(F("⚠️ LittleFS mount failed!"));
            // TODO: error management
        }

        if (!LittleFS.exists(CONFIG_FILE)) {
            // create default config
            Serial.println(F("⚠️ No config file, using defaults"));
            hostname = F("wfn-mask");
            universe = 1;
            channel = 1;
            for (int i = 0; i < MAX_LEDS; i++){ 
                leds[i].name = F("D0"); 
                leds[i].desc = F("LED");
                leds[i].pin = D0;
            }
            return;
        }
        File f = LittleFS.open(CONFIG_FILE, "r");
        StaticJsonDocument<512> doc;
        if (deserializeJson(doc, f)) {
            Serial.printf("Failed to parse %s\n", CONFIG_FILE);
            return;
        }
        f.close();

        hostname = doc["hostname"].as<String>();
        universe = doc["universe"];
        channel  = doc["channel"];
        for (int i = 0; i < MAX_LEDS; i++) {
            leds[i].name  = doc["leds"][i]["name"].as<String>();
            leds[i].desc = doc["leds"][i]["desc"].as<String>();
            leds[i].pin = _pin_from_string(leds[i].name);

            // Serial.printf("LOAD %s (%s / %d)\n", 
            //                     leds[i].desc.c_str(), 
            //                     leds[i].name.c_str(),
            //                     leds[i].pin);

        }
        // Serial.println("DeviceConfig CTOR OK");
    }


    void save() {
        StaticJsonDocument<512> doc;
        doc["hostname"] = hostname;
        doc["universe"] = universe;
        doc["channel"]  = channel;
        for (int i = 0; i < MAX_LEDS; i++) {
            doc["leds"][i]["name"] = leds[i].name;
            doc["leds"][i]["desc"] = leds[i].desc;
            leds[i].pin = _pin_from_string(leds[i].name);

            Serial.printf("SAVE %s %s (%d)\n", leds[i].desc.c_str(), 
                                               leds[i].name.c_str(),
                                               leds[i].pin);
        }

        File f = LittleFS.open(CONFIG_FILE, "w");
        if (!f) {
            Serial.printf("Failed to write %s\n", CONFIG_FILE);
            return;
        }
        serializeJson(doc, f);

        f.close();
        Serial.println("Config saved, reboot to apply");
    }
};