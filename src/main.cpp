
#include "secrets.h"
#include "version.h"
#include "device.h"
// #include "web_server.h"


// DeviceConfig config;
// ConfigWebServer web(config);


MaskDevice _device {};
void setup() {
    Serial.begin(9600);
    delay(1000);
    // _device = IDevice::instance();

    _device.begin();
}

void loop() {
    _device.tick();
    // _devicetick();

    // web.handleClient();
}
