MIDI2Artnet

Sous Windows 11:

- installer Reaper, LoopBe1, VSCode, Python 3.11

- sous VSCode, Terminal > New Terminal
- dans le terminal créer le virtaul env: `python -m venv venv`
- Activer le venv `venv\Scripts\Activate.ps1` (si Powershell bloque faire `Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass`)
- Install dépendances: `pip install mido python-rtmidi`
- Lancer: `python .\scripts\midi2artnet.py --broadcast 192.168.1.255 --universe 1`


Le script est multi channel sur son Universe, donc on peut créer plusieurs pistes midi sous reaper