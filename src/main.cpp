
#include "version.h"
#include "device.h"

#include "wifi_captive.h"

Device _device;
#define PIN_RESET_BUTTON (D7) // TODO: use D0 with a reistor tied to 3.3v, and a pushbutton tied to GND

void setup() {
    Serial.begin(9600);
    delay(100);

    Serial.printf("\nWFN-Device v%s - %s - %s\n", GIT_TAG, GIT_BRANCH, GIT_HASH);

    pinMode(PIN_RESET_BUTTON, INPUT_PULLUP /* TODO: use INPUT with D0 pin */);
    delay(100);
    if (digitalRead(PIN_RESET_BUTTON) == LOW) {
        // start captive portal
        Serial.println("Force config portal");
                
        WiFiCaptivePortal::start_captive_portal();

        // this is blocking until reboot
    }

    _device.begin();
}

void loop() {
    _device.tick();
}
