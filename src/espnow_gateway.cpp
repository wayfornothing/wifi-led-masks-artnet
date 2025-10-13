#include "crc.h"
#include "midi_packet.h"
#include "version.h"
#include "hal/hal.h"
// #include "espnow/gateway_captive.h"

#define MAX_CLIENTS 10
#define MAC_ADDR_DATA_SZ (6)


typedef struct {
    const char* name;
    uint8_t channel;
    uint8_t mac_address[MAC_ADDR_DATA_SZ];
    uint32_t seq;
} client_data_t;


client_data_t _client_data[] = {
    // {0x50, 0x78, 0x7D, 0x47, 0x1D, 0x48}, /* esp32 adafruit machin */
    {"Olivier", 2, { 0xEC, 0xFA, 0xBC, 0x63, 0x78, 0x31 }, 0},
    {"Yann",    3, { 0x8C, 0xAA, 0xB5, 0x16, 0x42, 0x1C }, 0},
    {"Jerome",  4, { 0x2C, 0xF4, 0x32, 0x7B, 0xE4, 0x71 }, 0},
    {"Maxime",  5, { 0xFC, 0xF5, 0xC4, 0x8B, 0x32, 0x50 }, 0},
    {"Bertie",  6, { 0x68, 0xC6, 0x3A, 0xF6, 0x51, 0x00 }, 0},
};

uint32_t _last_seq = 0;
uint32_t _recv_fails = 0;
uint32_t _lost_seqs = 0;
uint32_t _crc_fails = 0;

static void _on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status != ESP_NOW_SEND_SUCCESS) {
        _recv_fails++;
    }
}


void setup() {
    
    // config_reset_button();
    // delay_ms(100);
    // if (true || is_reset_button_pressed()) {
    //     // start captive portal
    //     Logger::info("Force config portal");
    //     CaptivePortal::start_captive_portal(); // this is blocking until reboot
    // }

    Serial.begin(115200);

    display_init();
    display_print_str("startup", 15, 25);
    delay(100);

    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        display_print_str("err 1", 15, 25);
        while (1);
    }
    if (esp_now_register_send_cb(_on_data_sent)) {
        display_print_str("err 2", 15, 25);
        while (1);
    }

    int idx = 0;
    for (const client_data_t& client: _client_data) {
        //for (auto address : _broadcast_addresses) {
        esp_now_peer_info_t peer_info;
        memcpy(peer_info.peer_addr, client.mac_address, 6);
        peer_info.channel = 0;
        peer_info.encrypt = false;
        if (esp_now_add_peer(&peer_info) != ESP_OK) {
            display_print_str("err 3", 15, 25);
        }
        else {
            display_print_str(client.name, 5, 25);

            // debug
            // static const int xOffset = 30; // = (132-w)/2
            // static const int yOffset = 12; // = (64-h)/2
            // u8g2.clearBuffer();
            // u8g2.setCursor(xOffset + 10, yOffset + 25);
            // u8g2.printf("reg ok: %d", ++idx);
            // u8g2.sendBuffer();
            delay(500);
        }
    }

    delay(2000);
    display_print_str("ready", 15, 25);
}


void loop() {
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

            for (client_data_t& client: _client_data) {
            // for (auto address : _broadcast_addresses) {
                if (MIDI_CHANNEL(pkt) + 1 == client.channel) {
                    pkt.seq = client.seq++;
                    pkt.crc = crc8_dallas((uint8_t *)&pkt, sizeof(midi_packet_t) - 1);
                    esp_now_send(client.mac_address, (uint8_t *)&pkt, sizeof(midi_packet_t));
                }
            }
            // esp_now_send(_broadcast_address, (uint8_t *)&pkt, sizeof(midi_packet_t));
        }
    }
}

