
# Programmation MIDI des LEDs

Chaque device peut assigner 7 LED Strips au maximum.

## Codes Program Change

Les Program Change permettent de sélectionner plusieurs LEDs à assigner (7 max).

Le code est un bitfield représentant les index des LEDs (7 maxi) concernées:

Bit | LED index
--- | ---  
0   | 1
1   | 2
2   | 3
etc...

Par exemple un PC#75 en binaire: 01001011, donc les LEDs 1, 2, 4 et 7 seront assignées.

Pour concerner toutes les LEDs, utiliser 01111111, soit PC#127.



## Codes Control Change

Si la valeur d'un Control Change est 0, la fonction correspondante est ignorée.

CC# | Code | Fonction | Valeur CC
--- | ---  | --- | ---
0 | CC_LED_OFF | Eteint la LEDs, annule les random / fade / blink, etc | Ignoré
1 | CC_LED_DIM | Envoie un signal PWM aux LEDs sélectionnées | Duty Cycle en % (plafonné à 100%)
2 | CC_LED_BLINK | Fait clignoter les LEDs | Intervalle entre 2 frames en millisecondes
3 | CC_LED_FADE_IN | Fondu entrant | Intervalle entre 2 frames en millisecondes
4 | CC_LED_FADE_OUT | Fondu sortant | Intervalle entre 2 frames en millisecondes
5 | CC_LED_HEARTBEAT | Pulsation continue | Intervalle entre 2 frames en millisecondes
6 | CC_LED_PULSE | Pulsation unique | Intervalle entre 2 frames en millisecondes
7 | CC_LED_RANDOM | Clignotement aléatoire jusqu'à annulation | Intervalle entre 2 frames en millisecondes
8 | CC_CFG_RANDOM_MID | Règle la balance du random | Balance en % (plafonné à 100%)
9 | CC_CFG_HEARTBEAT_MAX | Règle la luminosité max de HEARTBEAT et PULSE | 0 == éteint, 127 == luminosité max


## MIDI Notes

On peut programmer les LEDs avec les MIDI notes.

Chaque LED possède 10 fonctions (voir table Codes Control Change), donc pour choisir la LED à assigner, on ajoute (index de la LED * 10) au Code Control Change.

**Attention, l'index de la LED commence à 0 !**

Par exemple pour lancer un RANDOM sur la LED n°2:

```
Note: (led_index * 10) + code_cc_random = (2 * 10) + 7 ! 27
Velocity: 40 = chaque frame dure 40 millisecondes
```