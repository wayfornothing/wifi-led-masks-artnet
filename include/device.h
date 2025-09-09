#pragma once

#include <Arduino.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>

#include "secrets.h"
#include "device_config.h"
#include "web_server.h"
#include "wifi_manager.h"

#define TEST_BIT(v, b) (v & (1 << b))

class MaskDevice {

private:
    uint8_t                 _packet[600]; // enough for Art-Net header + DMX
    // const int               PACKET_SIZE = 32;
    WiFiManager             _wifi;
    DeviceConfig            _config;
    ConfigWebServer         _web;
    WiFiUDP                 _udp;
    const int               ARTNET_PORT = 6454;
    #define                 LED_CMD_SET     (0)
    #define                 LED_CMD_STROBE  (1)
    #define                 LED_CMD_FADE    (2)
    #define                 LED_CMD_RANDOM  (3)


    // LEDManager              _leds[MAX_LEDS];


public:
    MaskDevice() :
        _wifi(WIFI_SSID, WIFI_PASSWORD, 10'000, 2'000, 20, LED_BUILTIN),
        _web(ConfigWebServer(_config)) {
    }

    void begin() {
        // at startup, turn all leds OFF
        for (auto led : _config.leds) {
            pinMode(led.pin, OUTPUT);
            digitalWrite(led.pin, HIGH);
            Serial.printf("LED CONF %s (%s)\n", led.desc.c_str(), led.name.c_str());
        }


        _wifi.on_disconnect([=]() {
            Serial.println("WiFi disconnected! Will retry...");
            // network issue happened, turn all the pins ON
            for (auto led : _config.leds) {
                digitalWrite(led.pin, LOW);
            }
        });

        _wifi.on_connect([=]() {
            Serial.println("WiFi connected!");
            // network restored, turn all the pins OFF
            for (auto led : _config.leds) {
                digitalWrite(led.pin, HIGH);
            }
                        
            // load config + start web server
            _web.begin();

            // wait for artnet messages
            _wifi.set_hostname(_config.hostname.c_str());
            if (MDNS.begin(_config.hostname)) {
                Serial.print(F("mDNS responder started, browse to: "));
                Serial.print(F("http://"));
                Serial.print(_config.hostname);
                Serial.println(F(".local"));
            }

            _udp.begin(ARTNET_PORT);

            Serial.print(F("Device name: "));
            Serial.println(_config.hostname);
        });

        _wifi.on_connect_failed([=]() {
            // max connection attempts reached, turn all the pins ON
            for (auto led : _config.leds) {
                digitalWrite(led.pin, LOW);
            }
        });
    }


    void tick() {

        // keep service running
        MDNS.update();

        // check wifi status
        _wifi.tick();

        // run web server
        _web.tick();

        // process artnet packets
        if (_wifi.is_connected()) {

            int packet_size = _udp.parsePacket();
            if (packet_size > 0) {
                int len = _udp.read(_packet, sizeof(_packet)); // TODO: reduce size ?

                // Verify Art-Net header
                // 0   : "Art-Net\0"
                // 8-9 : OpCode (0x5000 for ArtDMX)
                // 10-11 : ProtVer
                // 12   : Sequence
                // 13   : Physical
                // 14-15: Universe (little endian)
                // 16-17: Length (big endian)
                // 18.. : DMX data (up to 512 bytes)

                if (len > 18 && memcmp(_packet, "Art-Net", 7) == 0) {
                    uint16_t opCode = _packet[8] | (_packet[9] << 8);
                    if (opCode == 0x5000)  { 
                        // OpOutput / ArtDmx
                        uint16_t universe = _packet[14] | (_packet[15] << 8);
                        uint16_t length = (_packet[16] << 8) | _packet[17];

                        if (universe == _config.universe && 
                            _config.channel <= length) {
                            // clamp buffer len
                            // if (length > 512) {
                            //     length = 512;
                            // }
                            uint8_t* dmx_data = _packet + 18;   // pointer to start of DMX data
                            uint8_t pc = dmx_data[_config.channel];
                            
                            // bit 6: enabled
                            bool enabled = TEST_BIT(pc, 6);

                            // bits 5 and 4: command
                            const uint8_t command = (pc >> 4) & 0x3;
                            Serial.printf("UNI %d CH %d PC %d CMD %d EN %d LEDs %d%d%d%d\n", universe, _config.channel, pc, command, enabled, 
                                            !!TEST_BIT(pc, 3), !!TEST_BIT(pc, 2), !!TEST_BIT(pc, 1), !!TEST_BIT(pc, 0));
                            
                            // bits [0..3]: LEDs
                            // TODO: optim
                            std::vector<uint8_t> pins;
                            for (int i = 0; i < MAX_LEDS; i++) {
                                if (TEST_BIT(pc, i)) {
                                    pins.push_back(_config.leds[i].pin);
                                }
                            }
                            
                            switch (command) {
                                case LED_CMD_SET: {
                                    enable(pins, enabled);
                                } break;
                                case LED_CMD_STROBE: {

                                } break;
                                case LED_CMD_FADE: {

                                } break;
                                case LED_CMD_RANDOM: {

                                } break;
                            }
                            
                            // process(dmx_data, length);
                        }
                        else {
                            // universe or channel not matching requisites
                            Serial.printf("Got universe %d\n", universe);
                        }
                    }
                    else {
                        // not 0x5000 opcode
                    }
                }
                else {
                    // not artnet header
                }
            }
            else {
                // empty packet
            }
        }
        else {
            // wifi disconnected
        }



        // process LEDs
        
    }


    void test(uint8_t pin) {
        digitalWrite(pin, LOW);
        delay(500);
        digitalWrite(pin, HIGH);
    }


    void strobe(std::vector<uint8_t> pins, bool enabled) {
        // for (auto pin : pins) {
        //     Serial.printf("STROBE pin=%d %dms\n", pin, duration_ms);

        // }
    }

    void fade(std::vector<uint8_t> pins, bool fade_in, uint32_t duration_ms) {
        for (auto pin : pins) {
            Serial.printf("FADE %s pin=%d %dms\n", fade_in ? "IN" : "OUT", pin, duration_ms);

        }
    }

    void random(std::vector<uint8_t> pins, uint32_t duration_ms) {
        for (auto pin : pins) {
            Serial.printf("RAND pin=%d %dms\n", pin, duration_ms);
        }
    }

    void enable(std::vector<uint8_t> pins, bool enabled) {
        // for (auto pin : pins) {
        for (int i = 0; i < MAX_LEDS; i++) {
            // Serial.printf("LED %d %s\n", pins[i], enabled ? "ON" : "OFF");
            digitalWrite(pins[i], enabled ?  LOW : HIGH);
        }
    }
};
