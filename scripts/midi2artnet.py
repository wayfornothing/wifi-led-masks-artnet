import mido
import socket
import argparse

ARTNET_PORT = 6454

TYPE_PC = 1
TYPE_CC = 2
TYPE_NOTE = 3

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
    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

    # choisir le port MIDI LoopBe
    port_name = None
    for name in mido.get_input_names():
        if "LoopBe Internal MIDI" in name:
            port_name = name
            break
    if not port_name:
        print("ERREUR: port MIDI 'LoopBe Internal MIDI' introuvable")
        return

    dmx = bytearray(512)
    seq = 1

    try:
        with mido.open_input(port_name) as inport:
            print("En attente de messages MIDI (Ctrl+C pour quitter)...")
            for msg in inport:
                channel = msg.channel  # 0–15
                base_index = channel * 3   # chaque canal MIDI occupe 3 slots DMX

                if msg.type == "program_change":
                    dmx[base_index] = TYPE_PC
                    dmx[base_index + 1] = msg.program
                    dmx[base_index + 2] = 0
                    print(f"[CH{channel+1}] ProgramChange {msg.program}")

                elif msg.type == "control_change":
                    dmx[base_index] = TYPE_CC
                    dmx[base_index + 1] = msg.control
                    dmx[base_index + 2] = msg.value
                    print(f"[CH{channel+1}] ControlChange {msg.control}:{msg.value}")

                elif msg.type == "note_on":
                    dmx[base_index] = TYPE_NOTE
                    dmx[base_index + 1] = msg.note
                    dmx[base_index + 2] = msg.velocity
                    print(f"[CH{channel+1}] NoteOn {msg.note} vel={msg.velocity}")

                elif msg.type == "note_off":
                    dmx[base_index] = TYPE_NOTE
                    dmx[base_index + 1] = msg.note
                    dmx[base_index + 2] = 0
                    print(f"[CH{channel+1}] NoteOff {msg.note}")

                packet = make_artnet_packet(args.universe, dmx, seq)
                sock.sendto(packet, (args.broadcast, ARTNET_PORT))
                seq = (seq + 1) % 256

    except KeyboardInterrupt:
        print("\nArrêt demandé par l'utilisateur")
    finally:
        sock.close()

if __name__ == "__main__":
    main()
