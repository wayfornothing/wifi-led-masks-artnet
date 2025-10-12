#pragma once

#include <unordered_map>
#include <functional>

#include "hal/hal.h"
#include "espnow/client_server.h"
#include "espnow/client_config.h"
#include "espnow/espnow_manager.h"
// #include "wifi_manager.h"
#include "led_device.h"
#include "midi_packet.h"
#include "crc.h"
#include "cc_commands.h"

#define TEST_BIT(v, b) (v & (1 << b))


static std::vector<LEDDevice>  _leds;
static uint32_t _crc_fails;
static uint32_t _last_seq;
static uint32_t _lost_seqs;


class MaskDevice {

private:
    const uint16_t          _dmx_base = (DeviceConfig::instance().get_channel() - 1) * 3;
    ESPNowManager           _network;

    static void _on_data_recv(uint8_t *mac, uint8_t *incoming_data, uint8_t len) {
        midi_packet_t pkt;
        if (len != sizeof(midi_packet_t)) {
            Logger::error("Wrong size: %d bytes\n", len);
            return;
        }

        memcpy(&pkt, incoming_data, sizeof(pkt));

        uint8_t calc = crc8_dallas((uint8_t*)&pkt, sizeof(pkt) - 1);
        if (calc != pkt.crc) {
            Logger::error("CRC FAIL (got %02X expected %02X)\n", pkt.crc, calc);
            _crc_fails++;
        }
        else {
            // Détection de perte
            if (pkt.seq != _last_seq + 1 && _last_seq != 0) {
                Logger::warn("⚠️  Missing packets: %lu → %lu\n", _last_seq, pkt.seq);
                _lost_seqs++;
            }
            _last_seq = pkt.seq;
            Logger::info("[%d/%d/%lu] type=%02X num=%d val=%d\n",
                           _crc_fails, _lost_seqs, pkt.seq, pkt.type, pkt.number, pkt.value);
            
            _process_midi(&pkt);
        }
    }

    static void _process_midi(midi_packet_t* pkt) {

        switch (pkt->type) {
            case MIDI_TYPE_PC:
                Logger::info("PC %d\n", pkt->number);
                _process_pc(pkt->number);
                break;

            case MIDI_TYPE_CC:
                Logger::info("CC %d val %d\n", pkt->number, pkt->value);
                if (pkt->value > 0) {
                    _process_cc(pkt->number, pkt->value);
                }
                break;

            case MIDI_TYPE_NOTE_OFF:
            case MIDI_TYPE_NOTE_ON: {
                uint8_t led_idx = pkt->number / CC_LAST;
                uint8_t cc = pkt->number % CC_LAST;
                Logger::info("Note%s %d vel %d idx %d cc %d\n", pkt->value == 0 ? "Off" : "On", pkt->number, pkt->value, led_idx, cc);
                if (led_idx < _leds.size()) {
                    if (pkt->value > 0) {
                        LEDDevice& led = _leds.at(led_idx);
                        led.select(true);
                        _process_cc(cc, pkt->value);
                        led.select(false);
                    }
                }
            } break;
            default:
                break;
    }
}

public:
    MaskDevice() : 
        _network(_on_data_recv) {
        _last_seq = _crc_fails = _lost_seqs = 0;
    }


    void tick() {
        _cli();
    }


    void begin() {
        DeviceConfig &_config = DeviceConfig::instance();

        // at startup, turn all leds OFF
        for (auto &led : _config.get_leds()) {
            Logger::info("Will create LED %s\n", led.name.c_str());
            _leds.push_back(led);
        }

        if (_network.begin() == false) {
            Logger::info("Turn ON all LEDs");
            for (LEDDevice &led : _leds)
            {
                led.enable(true);
            }
        }
        else {
            Logger::info("ESP-Now connected!");
            // network restored, turn all the pins OFF
            for (auto& led : _leds) {
                led.enable(false);
            }
        }
    }

private:

    static void _process_pc(uint8_t pc) {
        // Program Change selects LED indexes for future configuration
        int i = 0;
        for (auto &led : _leds) {
            led.select(TEST_BIT(pc, i));
            i++;
        }
    }

    static void _process_cc(uint8_t cc, uint8_t value) {
        static const std::unordered_map<uint8_t, std::function<void(LEDDevice&, uint8_t)>> cc_actions = {
            {CC_LED_OFF,            [](LEDDevice& led, uint8_t)  { led.enable(false);           }},
            {CC_LED_DIM,            [](LEDDevice& led, uint8_t v){ led.dim(v);                  }},
            {CC_LED_BLINK,          [](LEDDevice& led, uint8_t v){ led.blink(v);                }},
            {CC_LED_HEARTBEAT,      [](LEDDevice& led, uint8_t v){ led.heartbeat(v);            }},
            {CC_LED_PULSE,          [](LEDDevice& led, uint8_t v){ led.pulse(v);                }},
            {CC_LED_RANDOM,         [](LEDDevice& led, uint8_t v){ led.random(v);               }},
            {CC_LED_FADE_IN,        [](LEDDevice& led, uint8_t v){ led.fade_in(v);              }},
            {CC_LED_FADE_OUT,       [](LEDDevice& led, uint8_t v){ led.fade_out(v);             }},
            {CC_CFG_RANDOM_MID,     [](LEDDevice& led, uint8_t v){ led.set_random_midpoint(v);  }},
            {CC_CFG_HEARTBEAT_MAX,  [](LEDDevice& led, uint8_t v){ led.set_heartbeat_max(v);    }},
        };

        auto it = cc_actions.find(cc);
        if (it == cc_actions.end()) {
            return;
        }

        for (auto &led : _leds) {
            if (led.is_selected()) {
                it->second(led, value);
            }
        }
    }

    void _cli() {
        const uint8_t ALL_LEDS = 0x7f;
        int c = Serial.read();
        if (c >= 0) {
            _process_pc(ALL_LEDS);
            switch (c) {
            case 'e':
                _process_cc(CC_LED_OFF, 1);
                break;
            case 'B':
                _process_cc(CC_LED_BLINK, DEFAULT_BLINK_INTERVAL_MS);
                break;
            case 'R':
                _process_cc(CC_LED_RANDOM, DEFAULT_RANDOM_INTERVAL_MS);
                break;
            case 'F':
                _process_cc(CC_LED_FADE_IN, DEFAULT_FADE_INTERVAL_MS);
                break;
            case 'f':
                _process_cc(CC_LED_FADE_OUT, DEFAULT_FADE_INTERVAL_MS);
                break;
            case 'P':
            case 'p':
                _process_cc(CC_LED_PULSE, DEFAULT_FADE_INTERVAL_MS);
                break;
            case 'H':
            case 'h':
                _process_cc(CC_LED_HEARTBEAT, DEFAULT_FADE_INTERVAL_MS);
                break;
            }
        }
    }
};
