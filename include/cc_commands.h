#pragma once 

typedef enum CCCommand {
    CC_LED_OFF = 0,
    CC_LED_DIM,
    CC_LED_BLINK,
    CC_LED_FADE_IN,
    CC_LED_FADE_OUT,
    CC_LED_HEARTBEAT,
    CC_LED_PULSE,
    CC_LED_RANDOM,
    CC_CFG_RANDOM_MID,
    CC_CFG_HEARTBEAT_MAX,
    CC_LAST
} eCCCommand;