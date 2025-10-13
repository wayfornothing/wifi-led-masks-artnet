#pragma once

#include "hal/hal.h"

const char * CLIENTS_FILE = "/clients.json";

class MACLoader {
public:
    static std::vector<String>& load_mac_adresses() {
        std::vector<String> _macs;

        // load saved macs
        if (!fs_begin()) {
            // TODO: error mgmt
            Logger::error("MACS: LFS ERR");
            // TODO: error mgmt
            Logger::error("File system failed to mount after format!");
            return _macs;
        }

        // WIFI
        if (!fs_exists(CLIENTS_FILE)) {
            // TODO: error mgmt
            Logger::error("MACS: CFG ERR");
        }

        File macs_file = fs_open(CLIENTS_FILE, "r");
        if (!macs_file)
        {
            // TODO: error mgmt
            Logger::error("MACS: OPEN ERR");
            // return false;
        }
        DynamicJsonDocument macs_json(2048);
        DeserializationError err = deserializeJson(macs_json, macs_file);
        macs_file.close();
        if (err) {
            // TODO: error mgmt
            Logger::error("MACS: JSON ERR");
        }
        else {
            _macs.clear();
            if (macs_json.containsKey("clients")) {
                for (JsonObject mac_json : macs_json["clients"].as<JsonArray>()) {
                    // _macs.push_back(mac_json);
                    // Logger::info(mac_json);
                }
            }
        }
        return _macs;
    }
};