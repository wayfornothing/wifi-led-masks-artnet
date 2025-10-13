#!/usr/bin/env python3
import struct, serial, argparse, mido, time

def crc8_dallas(data: bytes) -> int:
    crc = 0
    for b in data:
        inbyte = b
        for _ in range(8):
            mix = (crc ^ inbyte) & 0x01
            crc >>= 1
            if mix:
                crc ^= 0x8C
            inbyte >>= 1
    return crc


def open_serial(port, bauds):
    """Try to open serial port safely."""
    while True:
        try:
            ser = serial.Serial(port, baudrate=bauds, timeout=0.1)
            print(f"[OK] Connected to {port}")
            return ser
        except serial.SerialException as e:
            print(f"[WARN] Serial open failed: {e}")
            time.sleep(0.1)


def make_packet(seq, midi_type, number, value):
    fmt = "<IBBB"  # seq, type, number, value
    base = struct.pack(fmt, seq, midi_type, number, value)
    crc = crc8_dallas(base)
    return base + struct.pack("<B", crc)

def list_midi_ports():
    for name in mido.get_input_names():
        print(" -", name)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", required=True)
    parser.add_argument("--midi", help="Nom du port MIDI virtuel")
    parser.add_argument("--list", action="store_true")
    parser.add_argument("--bauds", default=115200)
    args = parser.parse_args()

    if args.list:
        list_midi_ports()
        return

    ser = open_serial(args.port, args.bauds)
    midi_in = mido.open_input(args.midi)
    seq = 0

    print(f"Listening MIDI on '{args.midi}' and sending via {args.port}")
    while True:
        for msg in midi_in.iter_pending():
            data = msg.bytes()
            if len(data) == 2 and data[0] >= 0xc0:
                # PC
                pkt = make_packet(seq, data[0], data[1], 0xff)
            elif len(data) == 3:
                # CC, note, etc
                pkt = make_packet(seq, data[0], data[1], data[2])
            else:
                continue

            try: 
                ser.write(pkt)
            except serial.SerialException as e:
                print(f"[ERROR] Serial write failed: {e}")
                # Try to reconnect
                ser.close()
                ser = open_serial(args.port, args.bauds)
                continue
            
            print(f"[{seq:06}] {msg} -> {' '.join(f'{b:02X}' for b in pkt)}")
            seq += 1
        time.sleep(0.001)

if __name__ == "__main__":
    main()
