#pragma once

#include "hal/hal.h"
#include "crc.h"
#include "midi_packet.h"

#define MAX_CLIENTS 10
#define MAC_ADDR_DATA_SZ (6)

// esp_now_peer_info_t _peer_info;
uint8_t _broadcast_addresses[][MAC_ADDR_DATA_SZ] = {
    // {0x50, 0x78, 0x7D, 0x47, 0x1D, 0x48}, /* esp32 adafruit machin */
    {0x8C, 0xAA, 0xB5, 0x16, 0x42, 0x1C} /* yann */

};

// size_t _num_addresses = sizeof(_broadcast_addresses) / MAC_ADDR_DATA_SZ;

// uint8_t _broadcast_address[] = {0x50, 0x78, 0x7D, 0x47, 0x1D, 0x48};

uint32_t _last_seq = 0;
uint32_t _recv_fails = 0;

class GatewayDevice {
public:
    GatewayDevice() {
    }

    static void _on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
        if (status != ESP_NOW_SEND_SUCCESS) {
            _recv_fails++;
        }
    }

    void begin() {
        // Serial.begin(115200);

        // pinMode(LED_BUILTIN, OUTPUT);

        // load mac addresses
        

        display_init();
        display_print_str("startup", 15, 25);
        delay(100);

        WiFi.mode(WIFI_STA);
        if (esp_now_init() != ESP_OK) {
            display_print_str("err 1", 15, 25);
            while (1)
                ;
        }
        if (esp_now_register_send_cb(_on_data_sent)) {
            display_print_str("err 2", 15, 25);
            while (1)
                ;
        }

        // Register peer

        // memcpy(_peer_info.peer_addr, _broadcast_address, 6);
        // _peer_info.channel = 0;
        // _peer_info.encrypt = false;

        // Add peer        TODO: add peerS
        // if (esp_now_add_peer(&_peer_info) != ESP_OK) {
        //     display_print_str("err 3", 15, 25);
        //     while (1)
        //         ;
        // }

        for (auto address : _broadcast_addresses) {
            esp_now_peer_info_t peer_info;
            memcpy(peer_info.peer_addr, address, 6);
            peer_info.channel = 0;
            peer_info.encrypt = false;
            if (esp_now_add_peer(&peer_info) != ESP_OK) {
                display_print_str("err 3", 15, 25);
            }
        }

        delay(2000);

        display_print_str("ready", 15, 25);
    }

    uint32_t _lost_seqs = 0;
    uint32_t _crc_fails = 0;
    void tick() {
        static midi_packet_t pkt;
        if (Serial.available() >= sizeof(midi_packet_t)) {
            Serial.readBytes((char *)&pkt, sizeof(midi_packet_t));
            uint8_t calc = crc8_dallas((uint8_t *)&pkt, sizeof(midi_packet_t) - 1);
            if (calc != pkt.crc) {
                _crc_fails++;
            }
            else {
                if (pkt.seq != _last_seq + 1) {
                    ++_lost_seqs;
                }
                _last_seq = pkt.seq;

                // debug
                {
                    static const int xOffset = 30; // = (132-w)/2
                    static const int yOffset = 12; // = (64-h)/2

                    u8g2.clearBuffer();
                    u8g2.setCursor(xOffset + 10, yOffset + 25);
                    u8g2.printf("%d", pkt.seq);
                    u8g2.setCursor(xOffset + 10, yOffset + 40);
                    u8g2.printf("%d/%d/%d", _crc_fails, _lost_seqs, _recv_fails);
                    u8g2.sendBuffer();
                }


                for (auto address : _broadcast_addresses) {
                    esp_now_send(address, (uint8_t *)&pkt, sizeof(midi_packet_t));
                }

                // esp_now_send(_broadcast_address, (uint8_t *)&pkt, sizeof(midi_packet_t));
            }
        }
    }
};