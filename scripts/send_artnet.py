#!/usr/bin/env python3
import argparse
import socket
import struct
import sys
import time

# ✅ Program Change #44 on channel 11, universe 2:
# ./send_artnet.py --universe 2 --channel 11 --pc 44
# ✅ Control Change #12 with value 125 on channel 11, universe 2:
# ./send_artnet.py --universe 2 --channel 11 --cc 12 --value 125


def build_artnet_packet(universe, channel, pc=None, cc=None, value=None):
    # Art-Net OpCode = 0x5000 (ArtDmx)
    header = b'Art-Net\x00' + struct.pack('<H', 0x5000)
    protver = struct.pack('>H', 14)   # Protocol version 14
    seq, phy = 0, 0
    universe_bytes = struct.pack('<H', universe)
    length = struct.pack('>H', 512)

    dmx = bytearray([0] * 512)

    if pc is not None:
        if not (0 <= pc <= 127):
            sys.exit("Error: Program Change must be 0–127")
        dmx[channel - 1] = pc

    if cc is not None:
        if value is None:
            sys.exit("Error: Control Change requires --value")
        if not (0 <= cc <= 127):
            sys.exit("Error: Control Change number must be 0–127")
        if not (0 <= value <= 127):
            sys.exit("Error: Control Change value must be 0–127")
        dmx[channel - 1] = cc
        if channel < 512:
            dmx[channel] = value  # next slot

    packet = header + protver + bytes([seq, phy]) + universe_bytes + length + dmx
    return packet

def send_packet(packet, ip="255.255.255.255", port=6454):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.sendto(packet, (ip, port))
    sock.close()

def main():
    parser = argparse.ArgumentParser(description="Send raw Art-Net DMX packet (PC or CC)")
    parser.add_argument("--universe", type=int, required=True, help="Universe number")
    parser.add_argument("--channel", type=int, required=True, help="DMX channel (1-512)")
    parser.add_argument("--ip", default="255.255.255.255", help="Target IP (default broadcast)")

    parser.add_argument("--pc", type=int, help="Program Change number (0-127)")
    parser.add_argument("--cc", type=int, help="Control Change number (0-127)")
    parser.add_argument("--value", type=int, help="Control Change value (0-127)")
    parser.add_argument("--replay", type=int, help="Replay interval in milliseconds")

    args = parser.parse_args()

    if not (1 <= args.channel <= 512):
        sys.exit("Error: Channel must be 1–512")

    if args.pc is None and args.cc is None:
        sys.exit("Error: must specify either --pc or --cc")

    packet = build_artnet_packet(args.universe, args.channel, pc=args.pc, cc=args.cc, value=args.value)

    if args.replay:
        interval = args.replay / 1000.0
        print(f"Sending to {args.ip}, universe {args.universe}, channel {args.channel}, every {args.replay} ms (Ctrl+C to stop)")
        try:
            while True:
                send_packet(packet, args.ip)
                time.sleep(interval)
        except KeyboardInterrupt:
            print("\nStopped.")
    else:
        send_packet(packet, args.ip)
        print(f"Sent packet to {args.ip}, universe {args.universe}, channel {args.channel}")

if __name__ == "__main__":
    main()
