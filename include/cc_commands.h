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
    // random midpoint, blink midpoint, fade / heartbeat / pulse max value
    CC_CFG_SECONDARY,
    CC_CFG_RESERVED,
    CC_LAST
} eCCCommand;