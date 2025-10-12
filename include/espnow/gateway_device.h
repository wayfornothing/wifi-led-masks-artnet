#pragma once

#include <U8g2lib.h>
#include "hal/hal.h"
#include "crc.h"
#include "midi_packet.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6, 5);


#define MAX_CLIENTS 10


#define MAC_ADDR_DATA_SZ    (6)

esp_now_peer_info_t _peer_info;
uint8_t _broadcast_addresses[][MAC_ADDR_DATA_SZ] = {
    { 0x50, 0x78, 0x7D, 0x47, 0x1D, 0x48 },
    { 0x50, 0x78, 0x7D, 0x47, 0x1D, 0x48 },
    { 0x50, 0x78, 0x7D, 0x47, 0x1D, 0x48 },
    { 0x50, 0x78, 0x7D, 0x47, 0x1D, 0x48 }
};

size_t _num_addresses = sizeof(_broadcast_addresses) / MAC_ADDR_DATA_SZ;


uint8_t _broadcast_address[] = { 0x50, 0x78, 0x7D, 0x47, 0x1D, 0x48 };

uint32_t _last_seq = 0;
uint32_t _recv_fails = 0;


class GatewayDevice {
public:
    GatewayDevice() {

    }

    static const int xOffset = 30;
    static const int yOffset = 12;


    // there is no 72x40 constructor in u8g2 hence the 72x40 screen is mapped in the middle of the 132x64 pixel buffer of the SSD1306 controller
    const int width = 72;
    const int height = 40;

    void display_init()
    {
        u8g2.begin();
        u8g2.setContrast(255);    // set contrast to maximum
        u8g2.setBusClock(400000); // 400kHz I2C
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.clearBuffer();

    }


    void display_print_str(const char* str, int x, int y) {
        static const int xOffset = 30; // = (132-w)/2
        static const int yOffset = 12; // = (64-h)/2

        u8g2.clearBuffer(); // clear the internal memory
        u8g2.setCursor(xOffset + x, yOffset + y);
        u8g2.printf(str);
        u8g2.sendBuffer(); // transfer internal memory to the display
    }






static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // Serial.print("Envoi vers ");
    // for (int i=0; i<6; i++) Serial.printf("%02X", mac_addr[i]);
    // Serial.print(" -> ");
    // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAILED");
    if (status == ESP_NOW_SEND_SUCCESS) {
        _recv_fails++;
    }
    else {
        // display_print_str("   ", 25, 15);
        // digitalWrite(LED_BUILTIN, LOW);
    }
}

// void addClient(const uint8_t mac[6]) {
//     if (numClients < MAX_CLIENTS) {
//         memcpy(clientMACs[numClients], mac, 6);
//         numClients++;
//     }
// }


    void begin() {
    // Serial.begin(115200);

    // pinMode(LED_BUILTIN, OUTPUT);
    
        display_init();
        display_print_str("start", 15, 25);
        delay(100);
    
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        display_print_str("err 1", 15, 25);
        while (1);
    }
    if (esp_now_register_send_cb(onDataSent)) {
        display_print_str("err 2", 15, 25);
        while (1);
    }


    // Register peer

    memcpy(_peer_info.peer_addr, _broadcast_address, 6);
    _peer_info.channel = 0;  
    _peer_info.encrypt = false;

    // Add peer        TODO: add peerS
    if (esp_now_add_peer(&_peer_info) != ESP_OK){
        display_print_str("err 3", 15, 25);
        while(1);
    }

    display_print_str("ready", 15, 25);

    // Serial.println("Ready, you can launch python script");
    // Serial.end();
    // Serial.begin(115200);
}

// midi_packet_t _packet;

    uint32_t _lost_seqs = 0;
    uint32_t _crc_fails = 0;
    void tick() {

    static midi_packet_t pkt;
    if (Serial.available() >= sizeof(midi_packet_t)) {
        Serial.readBytes((char*)&pkt, sizeof(midi_packet_t));
        uint8_t calc = crc8_dallas((uint8_t*)&pkt, sizeof(midi_packet_t) - 1);
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
                u8g2.clearBuffer();
                u8g2.setCursor(xOffset + 10, yOffset + 25);
                u8g2.printf("%d", pkt.seq);
                u8g2.setCursor(xOffset + 10, yOffset + 40);
                u8g2.printf("%d/%d/%d", _crc_fails, _lost_seqs, _recv_fails);
                u8g2.sendBuffer();
            }
            esp_now_send(_broadcast_address, (uint8_t*)&pkt, sizeof(midi_packet_t));
        }
    }
}

};