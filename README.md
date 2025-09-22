
# ArtNet receiver

```bash
# send Program Change on Channel 2, universe 2
./scripts/send_artnet.py --universe 2 --channel 2 --pc 2
```

```
pc data bitfield

7  6  5  4  3  2  1  0

```

## Features 

- Push button on reset to enable captive portal (captive AP is `WFN-Config`)
- Auto-reconnect to configured WiFi network


## TODO



Commands per LED:

ENABLE
DISABLE
BLINK,interval
RANDOM,interval,midpoint
FADE_IN,interval,curve
FADE_OUT,interval,curve