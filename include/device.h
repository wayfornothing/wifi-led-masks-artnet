#pragma once

#include <Arduino.h>
#include <Ticker.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>

#include "secrets.h"
#include "device_config.h"
#include "web_server.h"
#include "wifi_manager.h"

#define TEST_BIT(v, b) (v & (1 << b))

class LEDDevice {

    class LEDManager {
    public:
        LEDManager(uint8_t pin) : _pin(pin), _state(false) {
            pinMode(_pin, OUTPUT);
            digitalWrite(_pin, LOW);
        }

        void start_blink(unsigned long intervalMs) {
            stop_blink(); // make sure any old timer is stopped
            _ticker.attach_ms(intervalMs, +[] (LEDManager* self) {
                self->_toggle();
            }, this);
        }

        void stop_blink() {
            _ticker.detach();
            digitalWrite(_pin, LOW);
            _state = false;
        }

        void enable(bool enabled) {
            _ticker.detach();
            digitalWrite(_pin, LOW);
            _state = false;
        }

    private:
        uint8_t _pin;
        bool _state;
        Ticker _ticker;

        void _toggle() {
            _state = !_state;
            digitalWrite(_pin, _state);
        }
    };

private:
    uint8_t                 _packet[600]; // enough for Art-Net header + DMX
    const int               PACKET_SIZE = 32;
    WiFiManager             _wifi;
    DeviceConfig            _config;
    ConfigWebServer         _web;
    WiFiUDP                 _udp;
    const int               ARTNET_PORT = 6454;


protected:
    // const int               _universe;
    // const int               _channel;
    // const char*             _name;
    // std::vector<uint8_t>    _pins;


public:
    // static Device* instance();
    // virtual ~Device() {};

    LEDDevice(/*const int universe, const int channel, const char* name, std::vector<uint8_t> pins*/) :
        _wifi(WIFI_SSID, WIFI_PASSWORD, 10'000, 5'000),
        //_universe(universe),
        _web(ConfigWebServer(_config)) {
    }

    void begin() {
        //_channel(channel),
        //_name(name),
        // _pins(pins) {
        // Serial.begin(9600);
        Serial.println("DEVICE BEGIN");
         

        // at startup, turn all leds OFF
        for (auto led : _config.leds) {
            pinMode(led.pin, OUTPUT);
            digitalWrite(led.pin, LOW);
            Serial.printf("LED CONF %s (%s)\n", led.desc.c_str(), led.name.c_str());
        }

        _wifi.on_disconnect([=]() {
            Serial.println("WiFi disconnected! Will retry...");
            // network issue happened, turn all the pins ON
            for (auto led : _config.leds) {
                digitalWrite(led.pin, HIGH);
            }
        });

        _wifi.on_connect([=]() {
            Serial.println("WiFi connected!");
            // network restored, turn all the pins OFF
            for (auto led : _config.leds) {
                digitalWrite(led.pin, LOW);
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


            // config is loaded
        });
    }


    // virtual int device_id() = 0;
    // const int channel() { return _channel; };
    // const int universe() { return _universe; };
    // const char* name() const { return _name; };

    // virtual void process(uint8_t * packet, uint16_t packet_len) = 0;

    typedef struct sPCMessage  {
        uint8_t reserved: 1;
        uint8_t enable: 1;
        uint8_t command: 2;
        uint8_t led_3: 1;
        uint8_t led_2: 1;
        uint8_t led_1: 1;
        uint8_t led_0: 1;
    } sPCMessage;

    typedef union {
        uint8_t     raw;
        // sPCMessage  pc;
        struct sPCMessage  {
            uint8_t reserved: 1;
            uint8_t enable: 1;
            uint8_t command: 2;
            uint8_t led_3: 1;
            uint8_t led_2: 1;
            uint8_t led_1: 1;
            uint8_t led_0: 1;
        } pc;
    } uPCMessage;

    typedef enum {
        SET = 0,
        STROBE,
        FADE,
        RANDOM
    } eCommand;


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
                int len = _udp.read(_packet, PACKET_SIZE); // TODO: reduce size ?

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
                            if (length > 512) {
                                length = 512;
                            }
                            uint8_t* dmx_data = _packet + 18;   // pointer to start of DMX data

                            // for (auto i=0; i < 32; i++) {
                            //     Serial.printf("0x%02x ", _packet[i]);
                            // }
                            uint8_t pc = dmx_data[_config.channel];
                            
                            uPCMessage upcm = {.raw = pc };

                            eCommand command = static_cast<eCommand>(upcm.pc.command >> 4);
                            bool enabled = upcm.pc.enable;
                            Serial.printf("Universe %d Channel %d PC %d CMD %d EN %d\n", universe, _config.channel, pc, command, enabled);
                            
                            std::vector<uint8_t> pins;
                            for (int i = 0; i < MAX_LEDS; i++) {
                                if (TEST_BIT(pc, i)) {
                                    // _config.leds[i].pin

                                    Serial.printf("PUSH %d %s (%s)\n", 
                                            _config.leds[i].pin, 
                                            _config.leds[i].desc.c_str(), 
                                            _config.leds[i].name.c_str());
                                    pins.push_back(_config.leds[i].pin);
                                }
                            }
                            
                            
                            switch (command) {
                                case SET: {
                                    enable(pins, enabled);
                                } break;
                                case STROBE: {

                                } break;
                                case FADE: {

                                } break;
                                case RANDOM: {

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
    }


    void test(uint8_t pin) {
        digitalWrite(pin, HIGH);
        delay(500);
        digitalWrite(pin, LOW);
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

            Serial.printf("LED %d %s\n", pins[i], enabled ? "ON" : "OFF");
            digitalWrite(pins[i], enabled ? HIGH : LOW);
        }
    }
};
