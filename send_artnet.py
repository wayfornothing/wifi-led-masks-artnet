#!/usr/bin/env python3
import argparse
import socket
import struct
import time
import sys

def build_artnet_packet(universe, channel, value):
    # Art-Net OpCode = 0x5000 (ArtDmx)
    header = b'Art-Net\x00' + struct.pack('<H', 0x5000)
    protver = struct.pack('>H', 14)   # Protocol version 14
    seq, phy = 0, 0
    universe_bytes = struct.pack('<H', universe)
    length = struct.pack('>H', 512)

    dmx = bytearray([0] * 512)
    dmx[channel - 1] = value  # channel is 1-based

    packet = header + protver + bytes([seq, phy]) + universe_bytes + length + dmx
    return packet

def send_packet(packet, ip="255.255.255.255", port=6454):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.sendto(packet, (ip, port))
    sock.close()

def main():
    parser = argparse.ArgumentParser(description="Send raw Art-Net DMX packet")
    parser.add_argument("--universe", type=int, required=True, help="Universe number")
    parser.add_argument("--channel", type=int, required=True, help="DMX channel (1-512)")
    parser.add_argument("--value", type=int, required=True, help="DMX value (0-255)")
    parser.add_argument("--ip", default="255.255.255.255", help="Target IP (default broadcast)")
    parser.add_argument("--replay", type=int, help="Replay interval in milliseconds")
    args = parser.parse_args()

    if not (1 <= args.channel <= 512):
        sys.exit("Error: Channel must be 1–512")
    if not (0 <= args.value <= 255):
        sys.exit("Error: Value must be 0–255")

    packet = build_artnet_packet(args.universe, args.channel, args.value)

    if args.replay:
        interval = args.replay / 1000.0
        print(f"Sending Art-Net packet to {args.ip}, universe {args.universe}, "
              f"channel {args.channel}={args.value}, every {args.replay} ms (Ctrl+C to stop)")
        try:
            while True:
                send_packet(packet, args.ip)
                time.sleep(interval)
        except KeyboardInterrupt:
            print("\nStopped.")
    else:
        send_packet(packet, args.ip)
        print(f"Sent Art-Net packet to {args.ip}, universe {args.universe}, "
              f"channel {args.channel}={args.value}")

if __name__ == "__main__":
    main()

