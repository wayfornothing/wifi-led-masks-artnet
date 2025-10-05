import mido
import socket
import argparse
import time
import signal
import sys

ARTNET_PORT = 6454

TYPE_PC = 1
TYPE_CC = 2
TYPE_NOTE = 3

stop_flag = False

def signal_handler(sig, frame):
    global stop_flag
    print("\n🛑 Stop (Ctrl+C)...")
    stop_flag = True

signal.signal(signal.SIGINT, signal_handler)


def make_artnet_packet(universe, dmx_data, seq=1):
    packet = bytearray()
    packet.extend(b'Art-Net\x00')
    packet.extend((0x00, 0x50))   # OpCode = ArtDMX
    packet.extend((0x00, 14))     # ProtVer
    packet.append(seq & 0xFF)
    packet.append(0x00)
    packet.extend(universe.to_bytes(2, 'little'))
    packet.extend(len(dmx_data).to_bytes(2, 'big'))
    packet.extend(dmx_data)
    return packet


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--universe", type=int, default=0)
    parser.add_argument("--broadcast", default="192.168.1.255")
    parser.add_argument("--filter_channel", type=int, default=0)
    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

    # TODO: win specific, add macOS / Linux support
    port_name = None
    for name in mido.get_input_names():
        if "LoopBe Internal MIDI" in name:
            port_name = name
            break
    if not port_name:
        print("❌ MIDI port 'LoopBe Internal MIDI' not found.")
        return

    dmx = bytearray(512)
    seq = 1

    with mido.open_input(port_name) as inport:
        print(f"✅ MIDI port: {port_name}")
        print("Waiting for MIDI messages (Ctrl+C to quit)...")

        while not stop_flag:
            for msg in inport.iter_pending():
                channel = msg.channel  # 0–15
                base_index = channel * 3   # chaque canal MIDI occupe 3 slots DMX

                channel += 1
                if args.filter_channel == 0 or args.filter_channel == channel:
                    if msg.type == "program_change":
                        dmx[base_index] = TYPE_PC
                        dmx[base_index + 1] = msg.program
                        dmx[base_index + 2] = 0
                        print(f"[CH{channel}] ProgramChange {msg.program}")

                    elif msg.type == "control_change":
                        dmx[base_index] = TYPE_CC
                        dmx[base_index + 1] = msg.control
                        dmx[base_index + 2] = msg.value
                        print(f"[CH{channel}] ControlChange {msg.control}:{msg.value}")

                    elif msg.type == "note_on":
                        dmx[base_index] = TYPE_NOTE
                        dmx[base_index + 1] = msg.note
                        dmx[base_index + 2] = msg.velocity
                        print(f"[CH{channel}] NoteOn {msg.note} vel={msg.velocity}")

                    elif msg.type == "note_off":
                        dmx[base_index] = TYPE_NOTE
                        dmx[base_index + 1] = msg.note
                        dmx[base_index + 2] = 0
                        print(f"[CH{channel}] NoteOff {msg.note}")

                    packet = make_artnet_packet(args.universe, dmx, seq)
                    sock.sendto(packet, (args.broadcast, ARTNET_PORT))
                    seq = (seq + 1) % 256

            time.sleep(0.01)  # permet d'interrompre proprement sur Ctrl+C

    sock.close()


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"❌ Error: {e}")
        sys.exit(1)
