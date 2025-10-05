
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
22 | CC_LED_OFF | Eteint les LEDs, annule aussi les random / fade / blink etc | Ignoré
23 | CC_LED_ON | Allume les LEDs | Ignoré
24 | CC_LED_DIM | Envoie un signal PWM aux LEDs sélectionnées | Duty Cycle en % (plafonné à 100%)
25 | CC_LED_BLINK | Fait clignoter les LEDs jusqu'à annulation | Intervalle en ms entre 2 frames
26 | CC_LED_FADE_IN | Fondu entrant | intervalle entre 2 frames (255 max) en millisecondes
27 | CC_LED_FADE_OUT | Fondu sortant | intervalle entre 2 frames (255 max) en millisecondes
28 | CC_LED_RANDOM | Clignotement aléatoire jusqu'à annulation | Intervalle en millisecondes entre 2 frames
29 | CC_CFG_RANDOM_MID | Règle la balance du random | Balance en % (plafonné à 100%). 0 == éteint, 100 == allumé

