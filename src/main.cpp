#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "secrets.h"
#include "version.h"

// ==== Art-Net settings ====
WiFiUDP Udp;
const int ARTNET_PORT = 6454;

// DMX buffer
uint8_t dmxBuffer[512];
int currentUniverse = -1;

// Which universe and channel to watch
const int WATCH_UNIVERSE = 2; // match your sender
const int WATCH_CHANNEL = 11; // match your sender

void setup()
{
    Serial.begin(9600);
    Serial.print("STARTUP");


    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" connected.");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    Udp.begin(ARTNET_PORT);
    Serial.printf("Listening for Art-Net on UDP port %d\n", ARTNET_PORT);
}

void loop()
{
    int packetSize = Udp.parsePacket();
    if (packetSize > 0)
    {
        uint8_t packet[600]; // enough for Art-Net header + DMX
        int len = Udp.read(packet, sizeof(packet));

        // Verify Art-Net header
        if (len > 18 && memcmp(packet, "Art-Net", 7) == 0)
        {
            uint16_t opCode = packet[8] | (packet[9] << 8);
            if (opCode == 0x5000)
            { // OpOutput / ArtDmx
                uint16_t incomingUniverse = packet[14] | (packet[15] << 8);
                uint16_t length = (packet[16] << 8) | packet[17];

                if (incomingUniverse == WATCH_UNIVERSE)
                {
                    // Copy DMX data into buffer
                    if (length > 512) {
                        length = 512;
                    }
                    memcpy(dmxBuffer, &packet[18], length);

                    int programValue = dmxBuffer[WATCH_CHANNEL - 1]; // DMX is 1-based

                    Serial.printf("Universe %d, Channel %d = Program Change %d\n",
                                    incomingUniverse, WATCH_CHANNEL, programValue);

                    // TODO: put your action here (switch preset, etc.)
                }
                else {
                    Serial.printf("Got universe %d\n", incomingUniverse);
                }
            }
        }
        else {
            Serial.printf("Not ArtNet header");
        }
    }
}
