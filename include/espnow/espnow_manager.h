#pragma once

#include "hal/hal.h"

class ESPNowManager {
private:
    esp_now_recv_cb_t _recv_cb;
public:
    ESPNowManager(esp_now_recv_cb_t recv_cb) : 
        _recv_cb(recv_cb) {
    }

    bool begin() {

        WiFi.mode(WIFI_STA);

        // Init ESP-NOW
        if (esp_now_init() != ESP_OK) {
            Logger::error("Error initializing ESP-NOW");

            // TODO: err mgmt
            // Logger::info("Turn ON all LEDs");
            // for (LEDDevice &led : _leds)
            // {
            //     led.enable(true);
            // }
            return false;
        }
        else
        {
            // esp-now OK
            if (esp_now_register_recv_cb(_recv_cb) != ESP_OK) {
                Logger::error("Error registering ESP-NOW");

                // TODO: err mgmt
                // Logger::info("Turn ON all LEDs");
                // for (LEDDevice &led : _leds)
                // {
                //     led.enable(true);
                // }
                return false;
            }
            else {
                // Logger::info("Client ready.");
                // display_print_str("ready", 15, 25);
                return true;
            }
        }
    }
};