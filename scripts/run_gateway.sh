 python3 -m venv venv
 ./venv/bin/activate
 source venv/bin/activate
 python3 -m pip install mido python-rtmidi pyserial
 ./scripts/midi2gateway.py --midi "IAC Driver Bus 1" --port /dev/tty.usbmodem1201