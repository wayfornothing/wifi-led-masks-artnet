
#include "hal/hal.h"

#include "espnow/client_device.h"
#include "espnow/client_captive.h"
#include "version.h"

MaskDevice _device;

void setup() {
    
    Logger::begin_hw();
    delay_ms(100);
    
    Logger::info("\nWFN-Device v%s - %s - %s\n", GIT_TAG, GIT_BRANCH, GIT_HASH);
    
    config_reset_button();
    delay_ms(100);
    if (false && is_reset_button_pressed()) {
        // start captive portal
        Logger::info("Force config portal");
        WiFiCaptivePortal::start_captive_portal(); // this is blocking until reboot
    }

    _device.begin();
}

void loop() {
    _device.tick();
}
