
#include "secrets.h"
#include "version.h"
#include "device.h"

IDevice * _device = nullptr;

void setup() {
    Serial.begin(9600);
    delay(1000);
    _device = IDevice::instance();

    Serial.print(F("Device name: "));
    Serial.println(_device->name());
}

void loop() {
    _device->tick();
}
