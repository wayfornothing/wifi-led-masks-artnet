#pragma once 

#include <stdint.h>

const uint8_t MIDI_TYPE_PC = 0xC0;
const uint8_t MIDI_TYPE_CC = 0xB0;
const uint8_t MIDI_TYPE_NOTE_ON = 0x90;
const uint8_t MIDI_TYPE_NOTE_OFF = 0x80;

#define MIDI_TYPE(type)    (type & 0xF0)
#define MIDI_CHANNEL(type) (type & 0x0F)

#pragma pack(push, 1)
typedef struct {
    uint32_t seq;
    uint8_t  type;
    uint8_t  number;
    uint8_t  value;
    uint8_t  crc;
} midi_packet_t;
#pragma pack(pop)
