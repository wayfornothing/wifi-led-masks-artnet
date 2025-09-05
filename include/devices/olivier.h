#pragma once

#ifdef DEVICE_OLIVIER

#include "device.h"

#define PIN_FOREHEAD_STRIP  D1
#define PIN_RIGHT_EYE_LED   D2
#define PIN_LEFT_EYE_LED    D3

class Device: public IDevice {

public:
    Device(int universe, int channel, const char* name, std::vector<uint8_t> pins)
        : IDevice(universe, channel, name, pins) {
    }

    void process(uint8_t * dmx_data, uint16_t dmx_data_len) {

        int pc = dmx_data[_channel];
        
        Serial.printf("%s process pc=%d\n", _name, pc);

        std::vector<uint8_t> pins;

        // bit 0: enable / disable
        bool enabled = TEST_BIT(pc, 0);

        // bit 1: forehead
        if (TEST_BIT(pc, 1)) {
            Serial.printf("forehead %s\n", enabled ? "ON" : "OFF");
            pins.push_back(PIN_FOREHEAD_STRIP);
        }

        // bit 2: left eye
        if (TEST_BIT(pc, 2)) {
            Serial.printf("left %s\n", enabled ? "ON" : "OFF");
            pins.push_back(PIN_LEFT_EYE_LED);
        }
        // bit 3: right eye
        if (TEST_BIT(pc, 3)) {
            Serial.printf("right %s\n", enabled ? "ON" : "OFF");
            pins.push_back(PIN_RIGHT_EYE_LED);
        }

        enable(pins, enabled);
    }
};

#endif












// #ifdef DEVICE_OLIVIER

// #define UNIVERSE            1
// #define DMX_CHANNEL         7
// #define PACKET_LENGTH       13
// #define PIN_FOREHEAD_STRIP  D1
// #define PIN_RIGHT_EYE_LED   D2
// #define PIN_LEFT_EYE_LED    D3

// class LEDDevice: public ILEDDevice {


// private:
//     void process_command(int pin, ELEDCommand command, uint8_t param1, uint8_t param2, uint8_t param3) {
        
//         dprint(wifi_hostname());
//         dprint(F(": processing command 0x"));
//         hprint(command);
//         dprint(F(", params 0x"));
//         hprint(param1);
//         dprint(F(", 0x"));
//         hprint(param2);
//         dprint(F(", 0x"));
//         hprint(param3);
        
//         switch (command) {
//             case ELEDCommandEnable: {
//                 // params: on/off
//                 bool enable = !!(param1 + param2 + param3);
//                 digitalWrite(pin, enable);
//             } break;
//             case ELEDCommandFadeIn: {
//                 uint32_t duration_ms = (param1 << 16) + (param2 << 8) + param3;
//                 // TODO:
//             } break;
//             case ELEDCommandFadeOut: {
//                 uint32_t duration_ms = (param1 << 16) + (param2 << 8) + param3;
//                 // TODO:

//             } break;
//             case ELEDCommandStrobe: {
//                 uint8_t strobe_ms = param1;
//                 uint16_t duration_ms = (param2 << 8) + param3;
//                 // TODO:

//             } break;
//             case ELEDCommandRandom: {
//                 uint32_t duration_ms = (param1 << 16) + (param2 << 8) + param3;
//                 // TODO:

//             } break;

//             default:
//             break;
//         }
//     }

// public:
//     LEDDevice() {
//         pinMode(PIN_FOREHEAD_STRIP, OUTPUT);
//         pinMode(PIN_RIGHT_EYE_LED, OUTPUT);
//         pinMode(PIN_LEFT_EYE_LED, OUTPUT);
//     }

//     const int dmx_channel() {
//         return DMX_CHANNEL;
//     }

//     const char* wifi_hostname() override { 
//         return (char const*) "olivier"; 
//     }


//     void wifi_connect() {
//         WiFi.hostname(wifi_hostname());
//         WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
//         dprint("Connecting to WiFi");
//         while (WiFi.status() != WL_CONNECTED) {
//             delay(500);
//             dprint(".");
//         }
//         dprintln();
//         dprint("Connected: ");
//         dprintln(WiFi.localIP().toString());
//     }
    

//     bool process_packet(uint8_t *data, size_t length) override {
//         bool ret = false;

//         dprint(wifi_hostname());
//         dprint(F(": raw command "));
//         for (size_t i = 0; i < length; i++) {
//             dprint("0x");
//             hprint(data[i]);
//             dprint(", ");
//         }
//         dprintln("");

//         if (/*data[0] == device_id() && */ length == PACKET_LENGTH) {
//             // this is for me !
//             process_command(PIN_FOREHEAD_STRIP, static_cast<ELEDCommand>(data[1]), data[2], data[3], data[4]);
//             process_command(PIN_RIGHT_EYE_LED, static_cast<ELEDCommand>(data[5]), data[6], data[7], data[8]);
//             process_command(PIN_LEFT_EYE_LED, static_cast<ELEDCommand>(data[9]), data[10], data[11], data[12]);
//         }
//         return ret;
//     }   

//     void on() override {
//         digitalWrite(PIN_RIGHT_EYE_LED, HIGH);
//         digitalWrite(PIN_LEFT_EYE_LED, HIGH);
//         digitalWrite(PIN_FOREHEAD_STRIP, HIGH);
//     }

//     void off() override {
//         digitalWrite(PIN_RIGHT_EYE_LED, LOW);
//         digitalWrite(PIN_LEFT_EYE_LED, LOW);
//         digitalWrite(PIN_FOREHEAD_STRIP, LOW);
//     }

//     void test() override {
//         static uint64_t ts = 0;
//         static bool enable = false;
//         if (millis() - ts > 500) {
//             if (enable) {
//                 on();
//             }
//             else {
//                 off();
//             }
//             ts = millis();
//             enable = !enable;
//         }
//     }
// };
// #endif