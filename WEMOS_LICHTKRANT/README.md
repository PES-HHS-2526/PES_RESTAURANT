# WEMOS Lichtkrant

## Beschrijving

Deze repository bevat de code voor de Wemos-lichtkrant binnen het PES restaurantproject.  
De module gebruikt een Wemos D1 Mini om een MAX7219 32x8 LED-matrix aan te sturen. Raspberry Pi-A kan via WiFi/TCP berichten naar de Wemos sturen, waarna deze berichten op de lichtkrant worden weergegeven.

De lichtkrant wordt gebruikt om informatie of meldingen zichtbaar te maken binnen het restaurantgedeelte van het domoticasysteem.

## Werking

De Wemos D1 Mini maakt verbinding met het WiFi-netwerk en start een TCP-server. Raspberry Pi-A kan als client verbinding maken met deze server en tekstberichten of commando's versturen.

Een normaal tekstbericht wordt direct weergegeven op de LED-matrix.

Voorbeeld:

```text
Welkom bij het restaurant
```

Daarnaast ondersteunt de lichtkrant speciale commando's.