#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "secrets.h"
#include "version.h"
#include "device.h"

// ==== Art-Net settings ====
WiFiUDP Udp;
const int ARTNET_PORT = 6454;

IDevice * _device = IDevice::instance();

void setup() {
    Serial.begin(9600);

    delay(1000);

    Serial.println("STARTUP");
    Serial.printf("%s\n", _device->name());
    Serial.print("Connecting to WiFi");


    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" connected.");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.printf("Device name: %s\n", _device->name());
    Serial.printf("Device universe: %d\n", _device->universe());
    Serial.printf("Device channel: %d\n", _device->channel());

    WiFi.hostname(_device->name());

    Udp.begin(ARTNET_PORT);
    Serial.printf("Listening for Art-Net on UDP port %d\n", ARTNET_PORT);
}

void loop()
{
    uint8_t packet[600]; // enough for Art-Net header + DMX
    int packetSize = Udp.parsePacket();
    if (packetSize > 0) {
        int len = Udp.read(packet, sizeof(packet)); // TODO: reduce size ?

        // Verify Art-Net header
// 0   : "Art-Net\0"
// 8-9 : OpCode (0x5000 for ArtDMX)
// 10-11 : ProtVer
// 12   : Sequence
// 13   : Physical
// 14-15: Universe (little endian)
// 16-17: Length (big endian)
// 18.. : DMX data (up to 512 bytes)

        if (len > 18 && memcmp(packet, "Art-Net", 7) == 0) {
            uint16_t opCode = packet[8] | (packet[9] << 8);
            if (opCode == 0x5000)  { 
                // OpOutput / ArtDmx
                uint16_t universe = packet[14] | (packet[15] << 8);
                uint16_t length = (packet[16] << 8) | packet[17];

                if (universe == _device->universe() && 
                    _device->channel() <= length) {
                    // Copy DMX data into buffer
                    if (length > 512) {
                        length = 512;
                    }
                    uint8_t* dmx_data = packet + 18;   // pointer to start of DMX data


                    // for (auto i=0; i < 32; i++) {
                    //     Serial.printf("0x%02x ", packet[i]);
                    // }


                    int pc = dmx_data[_device->channel()];

                    Serial.printf("Universe %d Channel %d PC %d\n", universe, _device->channel(), pc);

                    _device->process(dmx_data, length);
                }
                else {
                    Serial.printf("Got universe %d\n", universe);
                }
            }
        }
        else {
            Serial.printf("Not ArtNet header");
        }
    }
}
