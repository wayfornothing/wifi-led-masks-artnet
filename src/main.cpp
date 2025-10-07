
#include "hal/hal.h"

#include "mask_device.h"
#include "version.h"
#include "wifi_captive.h"

MaskDevice _device;
#define PIN_RESET_BUTTON (D0) // add a 10k resistor from D0 to 3v3R

void setup() {
    
    Logger::begin_hw();
    delay_ms(100);

    Logger::info("\nWFN-Device v%s - %s - %s\n", GIT_TAG, GIT_BRANCH, GIT_HASH);

    pin_set_input(PIN_RESET_BUTTON);
    delay_ms(100);
    if (pin_read_digital(PIN_RESET_BUTTON) == LOW) {
        // start captive portal
        Logger::info("Force config portal");
        WiFiCaptivePortal::start_captive_portal(); // this is blocking until reboot
    }

    _device.begin();
}

void loop() {
    _device.tick();
}
