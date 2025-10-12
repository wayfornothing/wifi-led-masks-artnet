// Arduino Code
// Works on, Boad type as: "Adafruit QT Py ESP32-C3"
// Dont forget to make sure that you downloaded and installed the board manager: "'esp32' by Espressif Systems"

#include "hal/hal.h"

#include "version.h"
#include "espnow/gateway_device.h"
// #include "espnow/gateway_captive.h"

GatewayDevice _device;

void setup() {
    
    Logger::begin_hw();
    delay_ms(100);
    
    Logger::info("\nWFN-Device v%s - %s - %s\n", GIT_TAG, GIT_BRANCH, GIT_HASH);
    
    // config_reset_button();
    // delay_ms(100);
    // if (true || is_reset_button_pressed()) {
    //     // start captive portal
    //     Logger::info("Force config portal");
    //     WiFiCaptivePortal::start_captive_portal(); // this is blocking until reboot
    // }

    _device.begin();
}


void loop() {
    _device.tick();
}

