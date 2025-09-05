#pragma once

#include <Arduino.h>
#include <Ticker.h>
#include <WiFiUdp.h>

#include "secrets.h"
#include "wifi_manager.h"

#define TEST_BIT(v, b) (v & (1 << b))

class IDevice {

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
    WiFiManager             _wifi;
    WiFiUDP                 _udp;
    const int               ARTNET_PORT = 6454;


protected:
    const int               _universe;
    const int               _channel;
    const char*             _name;
    std::vector<uint8_t>    _pins;


public:
    static IDevice* instance();
    virtual ~IDevice() {};

    IDevice(const int universe, const int channel, const char* name, std::vector<uint8_t> pins) :
        _wifi(WIFI_SSID, WIFI_PASSWORD, 10'000, 5'000),
        _universe(universe),
        _channel(channel),
        _name(name),
        _pins(pins) {

        // at startup, turn all leds OFF
        for (auto pin : _pins) {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, LOW);
        }

        _wifi.on_disconnect([pins]() {
            Serial.println("WiFi disconnected! Will retry...");
            // network issue happened, turn all the pins ON
            for (auto pin : pins) {
                digitalWrite(pin, HIGH);
            }
        });

        _wifi.on_connect([=]() {
            Serial.println("WiFi connected!");
            // network restored, turn all the pins OFF
            for (auto pin : pins) {
                digitalWrite(pin, LOW);
            }

            // wait for artnet messages
            _wifi.set_hostname(name);
            _udp.begin(ARTNET_PORT);
        });
    }


    // virtual int device_id() = 0;
    const int channel() { return _channel; };
    const int universe() { return _universe; };
    const char* name() const { return _name; };

    virtual void process(uint8_t * packet, uint16_t packet_len) = 0;

    void tick() {

        // check wifi status
        _wifi.tick();

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

                        if (universe == _universe && _channel <= length) {
                            // clamp buffer len
                            if (length > 512) {
                                length = 512;
                            }
                            uint8_t* dmx_data = _packet + 18;   // pointer to start of DMX data

                            // for (auto i=0; i < 32; i++) {
                            //     Serial.printf("0x%02x ", _packet[i]);
                            // }
                            int pc = dmx_data[_channel];
                            Serial.printf("Universe %d Channel %d PC %d\n", universe, _channel, pc);
                            process(dmx_data, length);
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
        for (auto pin : pins) {
            Serial.printf("LED %d %s\n", pin, enabled ? "ON" : "OFF");
            digitalWrite(pin, enabled ? HIGH : LOW);
        }
    }
};
