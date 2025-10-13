
#include "hal/hal.h"

#include "espnow/client_device.h"
#include "espnow/client_captive.h"
#include "version.h"

MaskDevice _device;

void setup() {
    
    Logger::begin_hw();
    delay_ms(100);
    
    Logger::info("\nWFN-Device v%s - %s - %s\n", GIT_TAG, GIT_BRANCH, GIT_HASH);
    
    WiFi.mode(WIFI_STA);
    Logger::info("MAC:%s ", WiFi.macAddress());


    config_reset_button();
    delay_ms(100);
    if (is_reset_button_pressed()) {
        // start captive portal
        Logger::info("Force config portal");
        CaptivePortal::start_captive_portal(); // this is blocking until reboot
    }

    _device.begin();
}

void loop() {
    _device.tick();
}
