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

typedef enum CCCommand {
    CC_LED_OFF = 22,
    CC_LED_ON,
    CC_LED_DIM,
    CC_LED_BLINK,
    CC_LED_FADE_IN,
    CC_LED_FADE_OUT,
    CC_LED_HEARTBEAT,
    CC_LED_RANDOM,
    CC_CFG_RANDOM_MID,
} eCCCommand;

#define TEST_BIT(v, b) (v & (1 << b))

class Device
{

private:
    const uint16_t          _dmx_base = (DeviceConfig::instance().get_channel() - 1) * 3;
    WiFiManager             _wifi;
    ConfigWebServer         _web;
    ArtnetWifi              _artnet;
    std::vector<LEDDevice>  _leds;

public:
    Device() : _wifi(10'000, 2'000, 20, LED_BUILTIN),
               _web(ConfigWebServer()) {
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
        _cli();

        // keep service running
        MDNS.update();

        // check wifi status
        _wifi.tick();

        // run web server
        _web.tick();

        // process artnet packets
        if (_wifi.is_connected()) {
            uint16_t opcode = _artnet.read();
            if (opcode == ART_DMX) {
                const uint8_t *dmx = _artnet.getDmxFrame();
                uint8_t type = dmx[_dmx_base];          // PC / CC / NOTE
                uint8_t number = dmx[_dmx_base + 1];    // #
                uint8_t value = dmx[_dmx_base + 2];     // value (except for PC)

                switch (type) {
                case MIDI_TYPE_PC:
                    Serial.printf("PC %d\n", number);
                    _process_pc(number);
                    break;

                case MIDI_TYPE_CC:
                    // Serial.printf("CC %d val %d\n", number, value);
                    if (value > 0) {
                        _process_cc(number, value);
                    }
                    break;

                case MIDI_TYPE_NOTE:
                    Serial.printf("Note%s %d vel %d\n", value == 0 ? "Off" : "On", number, value);
                    if (value > 0) {
                        _process_cc(number, value);
                    }
                    // else {
                    //     // disable all
                    //     _process_cc(CC_LED_OFF, 0);
                    // }
                    break;
                default:
                    break;
                }
            }
        }
        else {
            // wifi disconnected, should attempt to reconnect
        }
    }

private:

    void _process_pc(uint8_t pc) {
        // Program Change selects LED indexes for future configuration
        int i = 0;
        for (auto &led : _leds) {
            Serial.printf("LED %d (%s) %sSELECTED\n", i, led.name.c_str(), TEST_BIT(pc, i) ? "" : "UN");
            led.select(TEST_BIT(pc, i));
            i++;
        }
    }


    void _process_cc(uint8_t cc, uint8_t value) {
        static const std::unordered_map<uint8_t, std::function<void(LEDDevice&, uint8_t)>> cc_actions = {
            {CC_LED_ON,         [](LEDDevice& led, uint8_t)  { led.enable(true);            }},
            {CC_LED_OFF,        [](LEDDevice& led, uint8_t)  { led.enable(false);           }},
            {CC_LED_DIM,        [](LEDDevice& led, uint8_t v){ led.dim(v);                  }},
            {CC_LED_BLINK,      [](LEDDevice& led, uint8_t v){ led.start_blink(v);          }},
            {CC_LED_HEARTBEAT,  [](LEDDevice& led, uint8_t v){ led.start_heartbeat(v);      }},
            {CC_LED_RANDOM,     [](LEDDevice& led, uint8_t v){ led.start_random(v);         }},
            {CC_LED_FADE_IN,    [](LEDDevice& led, uint8_t v){ led.start_fade_in(v);        }},
            {CC_LED_FADE_OUT,   [](LEDDevice& led, uint8_t v){ led.start_fade_out(v);       }},
            {CC_CFG_RANDOM_MID, [](LEDDevice& led, uint8_t v){ led.set_random_midpoint(v);  }},
        };

        auto it = cc_actions.find(cc);
        if (it == cc_actions.end()) {
            return;
        }

        for (auto &led : _leds) {
            if (led.is_selected())
                it->second(led, value);
        }
    }




/*
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

    */
    void _cli()
    {
        uint8_t all_leds = 0x7f;
        int c = Serial.read();
        if (c >= 0) {
            _process_pc(all_leds);
            switch (c) {
            case 'E':
                // all ON
                _process_cc(CC_LED_ON, 1);
                break;
            case 'e':
                // all OFF
                _process_cc(CC_LED_OFF, 1);
                break;
            case 'S':
            case 'B':
                // blink ON
                _process_cc(CC_LED_BLINK, DEFAULT_BLINK_INTERVAL_MS);
                break;
            case 's':
            case 'b':
                // blink OFF
                _process_cc(CC_LED_OFF, 1);
                break;
            case 'R':
                // random ON
                _process_cc(CC_LED_RANDOM, DEFAULT_RANDOM_INTERVAL_MS);
                break;
            case 'r':
                // random OFF
                _process_cc(CC_LED_OFF, 1);
                break;
            case 'F':
                // fade IN
                _process_cc(CC_LED_FADE_IN, DEFAULT_FADE_INTERVAL_MS);
                break;
            case 'f':
                // fade OUT
                _process_cc(CC_LED_FADE_OUT, DEFAULT_FADE_INTERVAL_MS);
                break;
            case '*':
                // for (auto &led : _leds) {
                //     led.inc_random_interval_ms(5);
                //     led.start_random();
                // }
                break;
            case '/':
                // for (auto &led : _leds)
                // {
                //     led.inc_random_interval_ms(-5);
                //     led.start_random();
                // }
                break;
            case '+':
                // for (auto &led : _leds)
                // {
                //     led.inc_blink_interval_ms(5);
                //     led.start_blink();
                // }
                break;
            case '-':
                // for (auto &led : _leds)
                // {
                //     led.inc_blink_interval_ms(-5);
                //     led.start_blink();
                // }
                break;
            case 'm':
                // for (auto &led : _leds)
                // {
                //     led.inc_fade_interval_ms(1);
                // }
                break;
            case 'l':
                // for (auto &led : _leds)
                // {
                //     led.inc_fade_interval_ms(-1);
                // }
                break;
            }
        }
    }
};
