#pragma once

#include <Arduino.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>

#include "device_config.h"
#include "web_server.h"
#include "wifi_manager.h"
#include "led_device.h"

#define TEST_BIT(v, b) (v & (1 << b))

class Device {

private:
    uint8_t                 _packet[600]; // enough for Art-Net header + DMX
    // const int               PACKET_SIZE = 32;
    WiFiManager            _wifi;
    ConfigWebServer         _web;
    WiFiUDP                 _udp;
    std::vector<LEDDevice>  _leds;
    const int               ARTNET_PORT = 6454;
    #define                 LED_CMD_SET     (0)
    #define                 LED_CMD_BLINK   (1)
    #define                 LED_CMD_FADE    (2)
    #define                 LED_CMD_RANDOM  (3)
    
    public:
    Device() :
        _wifi(10'000, 2'000, 20, LED_BUILTIN),
        _web(ConfigWebServer()) {
    }
    
    void begin() {
        
        DeviceConfig& _config = DeviceConfig::instance();
        
        // at startup, turn all leds OFF
        for (auto& led_cfg : _config.get_leds()) {
            Serial.printf("Will create LED %s at pin %d\n", led_cfg.name.c_str(), led_cfg.pin);
            _leds.push_back(LEDDevice(led_cfg.pin, led_cfg.name));
        }

        _wifi.on_disconnect([=]() {
            Serial.println("WiFi disconnected! Will retry...");
            // network issue happened, turn all the pins ON
            for (auto& led : _leds) {
                led.enable(true);
            }
        });

        _wifi.on_connect([&]() {
            Serial.println("WiFi connected!");
            // network restored, turn all the pins OFF
            for (auto& led : _leds) {
                led.enable(false);
            }
                        
            // load config + start web server
            _web.begin();

            // wait for artnet messages
            // DeviceConfig& cfg = DeviceConfig::instance();
            const String hostname = _config.get_hostname();
            _wifi.set_hostname(hostname.c_str());
            if (MDNS.begin(hostname)) {
                Serial.print(F("mDNS responder started, browse to: "));
                Serial.print(F("http://"));
                Serial.print(hostname);
                Serial.println(F(".local"));
            }

            _udp.begin(ARTNET_PORT);
        });

        _wifi.on_connect_failed([=]() {
            // max connection attempts reached, turn all the pins ON
            for (auto& led : _leds) {
                led.enable(true);
            }
        });
    }

    void tick() {

        _cli();

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

                        DeviceConfig& cfg = DeviceConfig::instance();
                        if (universe == cfg.get_universe() && 
                            cfg.get_channel() <= length) {
                            // clamp buffer len
                            // if (length > 512) {
                            //     length = 512;
                            // }
                            uint8_t* dmx_data = _packet + 18;   // pointer to start of DMX data
                            uint8_t pc = dmx_data[cfg.get_channel()];
                            
                            // bit 6: enabled
                            bool enabled = TEST_BIT(pc, 6);

                            // bits 5 and 4: command
                            const uint8_t command = (pc >> 4) & 0x3;

                            Serial.printf("UNI %d CH %d PC %d CMD %d EN %d LEDs %d%d%d%d\n", universe, cfg.get_channel(), pc, command, enabled, 
                                            !!TEST_BIT(pc, 3), !!TEST_BIT(pc, 2), !!TEST_BIT(pc, 1), !!TEST_BIT(pc, 0));
                            _process(pc & 0xF, command, enabled);
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
    }

private:
    void _process(uint8_t leds_bitfield, uint8_t command, bool enabled) {
        switch (command) {
            case LED_CMD_SET: {
                int i = 0;
                for (auto& led : _leds) {
                    if (TEST_BIT(leds_bitfield, i)) {
                        Serial.printf("SET LED %d (%s) %s\n", i, led.name.c_str(), enabled ? "ON" : "OFF");
                        led.enable(enabled);
                    }
                    i++;
                }
            } break;
            case LED_CMD_BLINK: {
                if (enabled) {
                    int i = 0;
                    for (auto& led : _leds) {
                        if (TEST_BIT(leds_bitfield, i)) {
                            Serial.printf("BLINK LED %d (%s) %s\n", i, led.name.c_str(), enabled ? "ON" : "OFF");
                            led.start_blink();
                        }
                        i++;
                    }
                }
            } break;
            case LED_CMD_FADE: {
                if (enabled) {
                    // fade in
                    int i = 0;
                    for (auto& led : _leds) {
                        if (TEST_BIT(leds_bitfield, i)) {
                            Serial.printf("FADE IN LED %d (%s) %s\n", i, led.name.c_str(), enabled ? "ON" : "OFF");
                            led.start_fade_in();
                        }
                        i++;
                    }
                }
                else {
                    // fade out
                    // Serial.println(F("FADE OUT"));
                    int i = 0;
                    for (auto& led : _leds) {
                        if (TEST_BIT(leds_bitfield, i)) {
                            Serial.printf("FADE OUT LED %d (%s) %s\n", i, led.name.c_str(), enabled ? "ON" : "OFF");
                            led.start_fade_out();
                        }
                        i++;
                    }
                }
            } break;
            case LED_CMD_RANDOM: {
                if (enabled) {
                    // random start
                    int i = 0;
                    for (auto& led : _leds) {
                        if (TEST_BIT(leds_bitfield, i)) {
                            Serial.printf("RANDOM OUT LED %d (%s) %s\n", i, led.name.c_str(), enabled ? "ON" : "OFF");
                            led.start_random();
                        }
                        i++;
                    }
                }
            } break;
        }
    }

    void _cli() {
        uint8_t all_leds = 0xF;
        int c = Serial.read();
        if (c >= 0) {
            switch (c) {
                case 'E':
                    // all ON
                    _process(all_leds, LED_CMD_SET, true);
                    break;
                case 'e':
                    // all OFF
                    _process(all_leds, LED_CMD_SET, false);
                    break;
                case 'S':
                case 'B':
                    // blink ON
                    _process(all_leds, LED_CMD_BLINK, true);
                    break;
                case 's':
                case 'b':
                    // blink OFF
                    _process(all_leds, LED_CMD_SET, true);
                break;
                case 'R':
                    // random ON
                    _process(all_leds, LED_CMD_RANDOM, true);
                    break;
                case 'r':
                    // random OFF
                    _process(all_leds, LED_CMD_SET, true);
                    break;
                case 'F':
                    // fade IN
                    _process(all_leds, LED_CMD_FADE, true);
                    break;
                case 'f':
                    // fade OUT
                    _process(all_leds, LED_CMD_FADE, false);
                    break;
                case '*':
                    for (auto& led : _leds) {
                        led.random_interval_ms += 10;
                        Serial.printf("RITV: %dms\n", led.random_interval_ms);
                        led.start_random();
                    }
                    break;
                case '/':
                    for (auto& led : _leds) {
                        led.random_interval_ms -= 10;
                        Serial.printf("RITV: %dms\n", led.random_interval_ms);
                        led.start_random();
                    }
                    break;
                case '+':
                    for (auto& led : _leds) {
                        led.blink_interval_ms += 10;
                        Serial.printf("BITV: %dms\n", led.blink_interval_ms);
                        led.start_blink();
                    }
                    break;
                case '-':
                    for (auto& led : _leds) {
                        led.blink_interval_ms -= 10;
                        Serial.printf("BITV: %dms\n", led.blink_interval_ms);
                        led.start_blink();
                    }
                    break;

            }
        }
    }
};
