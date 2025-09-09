
#include "secrets.h"
#include "version.h"
#include "device.h"

Device _device {};

LEDDevice led1(D1, "D1");
LEDDevice led2(D2, "D2");
LEDDevice led3(D3, "D2");
LEDDevice led4(LED_BUILTIN, "D4");

// std::vector<LEDDevice> _leds = {led1, led2, led3, led4};

void setup() {
    Serial.begin(9600);
    delay(1000);
    // for (auto& led: _leds) {
    //     led.start_blink();
    // }

    _device.begin();
}

void loop() {
    _device.tick();


    // switch (Serial.read()) {
    //     case '*':
    //         for (auto& led : _leds) {
    //             led.random_interval_ms += 10;
    //             Serial.printf("ITV: %dms\n", led.random_interval_ms);
    //             led.start_random();
    //         } break;
    //     case '/':
    //         for (auto& led : _leds) {
    //             led.random_interval_ms -= 10;
    //             Serial.printf("ITV: %dms\n", led.random_interval_ms);
    //             led.start_random();
    //         } break;
    //     case '+':
    //         for (auto& led : _leds) {
    //             led.blink_interval_ms += 10;
    //             Serial.printf("ITV: %dms\n", led.blink_interval_ms);
    //             led.start_blink();
    //         } break;
    //     case '-':
    //         for (auto& led : _leds) {
    //             led.blink_interval_ms -= 10;
    //             Serial.printf("ITV: %dms\n", led.blink_interval_ms);
    //             led.start_blink();
    //         } break;
    //     case 'b':
    //         for (auto& led : _leds) {
    //             led.enable(true);
    //         } break;
    //     case 'B':
    //         for (auto& led : _leds) {
    //             led.start_blink();
    //         } break;
    //     case 'r':
    //         for (auto& led : _leds) {
    //             led.enable(true);
    //         } break;
    //     case 'R':
    //         for (auto& led : _leds) {
    //             led.start_random();
    //         } break;
    //     case 'E':
    //         for (auto& led : _leds) {
    //             led.enable(true);
    //         } break;
    //     case 'e':
    //         for (auto& led : _leds) {
    //             led.enable(false);
    //         } break;
    //     case 'f':
    //         for (auto& led : _leds) {
    //             led.start_fade_out();
    //         } break;
    //     case 'F':
    //         for (auto& led : _leds) {
    //             led.start_fade_in();
    //         } break;
    // }
}
