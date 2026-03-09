## Gateway

Projet ESP32 qui lit des commandes MIDI depuis un PC par le port USB, et les broadcaste vers les Clients avec le protocole ESP-Now.

### macOS

- Lancer l'utilitaire macOS "Configuration audio et MIDI"
- Menu Fenêtre > Afficher le studio MIDI
- Double clic sur la box "Gestionnaire IAC" (ou "IAC Driver"), si aucune box cliquer sur l'icone "Réinitialiser"
- Ajouter un port si nécessaire, le "Bus 1" par défaut convient.

Installer les prérequis:

```bash
cd scripts
python3 -m venv .venv
source .venv/bin/activate
pip3 install pyserial mido python-rtmidi
```

Lister les ports disponibles:

```bash
./midi2gateway.py --list
```

Trouver le port du Gateway:

```bash
ll /dev/tty.usb*
```

Lancer la commande (exemple):

```bash
./midi2gateway.py --midi "IAC Driver Bus 1" --port /dev/tty.usbmodem21201
```

Envoyer des commandes MIDI:

Avec Reaper, Logic Pro, GarageBand etc, sinon en CLI:

```bash
# https://github.com/gbevin/SendMIDI
brew install gbevin/tools/sendmidi
sendmidi dev "IAC Driver Bus 1" pc 0
```


### Windows 

TODO

## Clients

Projet ESP32 qui lit les commandes MIDI envoyées par le Gateway avec le protocole ESP-Now.

Maintenir le bouton CAPTIVE appuyé au reset (ou power-up) pour lancer le device en mode Captif/Configuration: on peut alors se connecter à son AP WiFi (WFN-Config), et configurer les sorties, et récupérer la MAC Address.