#pragma once

#include <Arduino.h>
// #include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <ArtnetWifi.h>

#include "device_config.h"
#include "web_server.h"
#include "wifi_manager.h"
#include "led_device.h"

#define MIDI_TYPE_PC (1)
#define MIDI_TYPE_CC (2)
#define MIDI_TYPE_NOTE (3)

enum CCCommands
{
    CC_LED_OFF = 22,
    CC_LED_ON,
    CC_LED_BLINK,
    CC_LED_FADE_IN,
    CC_LED_FADE_OUT,
    CC_LED_RANDOM,
    CC_CFG_BLINK_INT,
    CC_CFG_RANDOM_INT,
    CC_CFG_RANDOM_MID,
    CC_CFG_FADE_IN_INT,
    CC_CFG_FADE_OUT_INT,
};

#define TEST_BIT(v, b) (v & (1 << b))

class Device
{

private:
    const uint16_t _dmx_base = (DeviceConfig::instance().get_channel() - 1) * 2;

    uint8_t _packet[600]; // enough for Art-Net header + DMX
    // const int               PACKET_SIZE = 32;
    WiFiManager _wifi;
    ConfigWebServer _web;
    // WiFiUDP                 _udp;
    ArtnetWifi _artnet;
    std::vector<LEDDevice> _leds;
    const int ARTNET_PORT = 6454;

public:
    Device() : _wifi(10'000, 2'000, 20, LED_BUILTIN),
               _web(ConfigWebServer())
    {
    }

    void begin()
    {

        DeviceConfig &_config = DeviceConfig::instance();

        // at startup, turn all leds OFF
        for (auto &led : _config.get_leds())
        {
            Serial.printf("Will create LED %s\n", led.name.c_str());

            // LEDDevice led = LEDDevice(led_cfg.pin, led_cfg.name);
            // led.blink_interval_ms = led_cfg.blink_ms;
            // led.random_interval_ms = led_cfg.random_ms;
            // led.random_midpoint = led_cfg.random_midpoint;
            // led.fade_interval_ms = led_cfg.fade_ms;
            _leds.push_back(led);
        }

        _wifi.on_disconnect([=]()
                            {
            Serial.println("WiFi disconnected! Will retry...");
            // network issue happened, turn all the pins ON
            for (auto& led : _leds) {
                led.enable(true);
            } });

        _wifi.on_connect([&]()
                         {
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

            // _udp.begin(ARTNET_PORT);
            _artnet.begin(); });

        _wifi.on_connect_failed([=]()
                                {
            // max connection attempts reached, turn all the pins ON
            for (auto& led : _leds) {
                led.enable(true);
            } });
    }

    void tick()
    {
        // static uint8_t prev_pc = 0;
        // static uint8_t prev_cc = 0;
        // static uint8_t prev_note = 0;
        _cli();

        // keep service running
        MDNS.update();

        // check wifi status
        _wifi.tick();

        // run web server
        _web.tick();

        // process artnet packets
        if (_wifi.is_connected())
        {
            uint16_t opcode = _artnet.read();
            if (opcode == ART_DMX)
            {
                const uint8_t *dmx = _artnet.getDmxFrame();
                uint8_t type = dmx[_dmx_base];
                uint8_t command = dmx[_dmx_base + 1];
                uint8_t value = dmx[_dmx_base + 2];

                switch (type)
                {
                case MIDI_TYPE_PC:
                    Serial.printf("PC %d\n", command);
                    break;

                case MIDI_TYPE_CC:
                    Serial.printf("CC %d val %d\n", command, value);
                    _process(command, value);
                    break;

                case MIDI_TYPE_NOTE:
                    Serial.printf("Note%s %d vel %d\n", value == 0 ? "Off" : "On", command, value);
                    break;

                default:
                    // rien reçu
                    break;
                }
            }
        }
        else
        {
            // wifi disconnected
        }
    }

private:
    void _process(uint8_t cc, uint8_t value) {
        int i = 0;
        switch (cc) {
            case CC_LED_ON: {
                for (auto &led : _leds) {
                    if (TEST_BIT(value, i)) {
                        Serial.printf("LED %d (%s) ON\n", i, led.name.c_str());
                        led.enable(true);
                    }
                    i++;
                }
            }
            break;
            case CC_LED_OFF: {
                for (auto &led : _leds) {
                    if (TEST_BIT(value, i)) {
                        Serial.printf("LED %d (%s) OFF\n", i, led.name.c_str());
                        led.enable(false);
                    }
                    i++;
                }
            }
            break;
            case CC_LED_BLINK: {
                for (auto &led : _leds) {
                    if (TEST_BIT(value, i)) {
                        Serial.printf("LED %d (%s) BLINK\n", i, led.name.c_str());
                        led.start_blink();
                    }
                    i++;
                }
            }
            break;
            case CC_LED_RANDOM: {
                for (auto &led : _leds) {
                    if (TEST_BIT(value, i)) {
                        Serial.printf("LED %d (%s) RAND\n", i, led.name.c_str());
                        led.start_random();
                    }
                    i++;
                }
            }
            break;
            case CC_LED_FADE_IN: {
                for (auto &led : _leds) {
                    if (TEST_BIT(value, i)) {
                        Serial.printf("LED %d (%s) FADE IN\n", i, led.name.c_str());
                        led.start_fade_in();
                    }
                    i++;
                }
            }
            break;
            case CC_LED_FADE_OUT: {
                for (auto &led : _leds) {
                    if (TEST_BIT(value, i)) {
                        Serial.printf("LED %d (%s) FADE OUT\n", i, led.name.c_str());
                        led.start_fade_out();
                    }
                    i++;
                }
            }
            break;
            case CC_CFG_BLINK_INT: {
            }
            break;
            case CC_CFG_FADE_IN_INT: {
            }
            break;
            case CC_CFG_FADE_OUT_INT: {
            }
            break;
            case CC_CFG_RANDOM_INT: {
            }
            break;
            case CC_CFG_RANDOM_MID: {
            }
            break;
        }
    }

    void _process(uint8_t leds_bitfield, uint8_t command, bool enabled)
    {
        switch (command)
        {
        case CC_LED_ON:
        {
            int i = 0;
            for (auto &led : _leds)
            {
                if (TEST_BIT(leds_bitfield, i))
                {
                    // Serial.printf("SET LED %d (%s) %s\n", i, led.name.c_str(), enabled ? "ON" : "OFF");
                    led.enable(enabled);
                }
                i++;
            }
        }
        break;
        case CC_LED_BLINK:
        {
            if (enabled)
            {
                int i = 0;
                for (auto &led : _leds)
                {
                    if (TEST_BIT(leds_bitfield, i))
                    {
                        // Serial.printf("BLINK LED %d (%s) %s\n", i, led.name.c_str(), enabled ? "ON" : "OFF");
                        led.start_blink();
                    }
                    i++;
                }
            }
        }
        break;
        case CC_LED_FADE_IN:
        {
            if (enabled)
            {
                // fade in
                int i = 0;
                for (auto &led : _leds)
                {
                    if (TEST_BIT(leds_bitfield, i))
                    {
                        // Serial.printf("FADE IN LED %d (%s) %s\n", i, led.name.c_str(), enabled ? "ON" : "OFF");
                        led.start_fade_in();
                    }
                    i++;
                }
            }
            else
            {
                // fade out
                // Serial.println(F("FADE OUT"));
                int i = 0;
                for (auto &led : _leds)
                {
                    if (TEST_BIT(leds_bitfield, i))
                    {
                        // Serial.printf("FADE OUT LED %d (%s) %s\n", i, led.name.c_str(), enabled ? "ON" : "OFF");
                        led.start_fade_out();
                    }
                    i++;
                }
            }
        }
        break;
        case CC_LED_RANDOM:
        {
            if (enabled)
            {
                // random start
                int i = 0;
                for (auto &led : _leds)
                {
                    if (TEST_BIT(leds_bitfield, i))
                    {
                        // Serial.printf("RANDOM OUT LED %d (%s) %s\n", i, led.name.c_str(), enabled ? "ON" : "OFF");
                        led.start_random();
                    }
                    i++;
                }
            }
        }
        break;
        }
    }

    void _cli()
    {
        uint8_t all_leds = 0xff;
        int c = Serial.read();
        if (c >= 0)
        {
            switch (c)
            {
            case 'E':
                // all ON
                _process(all_leds, CC_LED_ON);
                break;
            case 'e':
                // all OFF
                _process(all_leds, CC_LED_OFF);
                break;
            case 'S':
            case 'B':
                // blink ON
                _process(all_leds, CC_LED_BLINK, true);
                break;
            case 's':
            case 'b':
                // blink OFF
                _process(all_leds, CC_LED_OFF);
                break;
            case 'R':
                // random ON
                _process(all_leds, CC_LED_RANDOM, true);
                break;
            case 'r':
                // random OFF
                _process(all_leds, CC_LED_OFF);
                break;
            case 'F':
                // fade IN
                _process(all_leds, CC_LED_FADE_IN);
                break;
            case 'f':
                // fade OUT
                _process(all_leds, CC_LED_FADE_OUT);
                break;
            case '*':
                for (auto &led : _leds)
                {
                    led.inc_random_interval_ms(5);
                    led.start_random();
                }
                break;
            case '/':
                for (auto &led : _leds)
                {
                    led.inc_random_interval_ms(-5);
                    led.start_random();
                }
                break;
            case '+':
                for (auto &led : _leds)
                {
                    led.inc_blink_interval_ms(5);
                    led.start_blink();
                }
                break;
            case '-':
                for (auto &led : _leds)
                {
                    led.inc_blink_interval_ms(-5);
                    led.start_blink();
                }
                break;
            case 'm':
                for (auto &led : _leds)
                {
                    led.inc_fade_interval_ms(1);
                }
                break;
            case 'l':
                for (auto &led : _leds)
                {
                    led.inc_fade_interval_ms(-1);
                }
                break;
            }
        }
    }
};
