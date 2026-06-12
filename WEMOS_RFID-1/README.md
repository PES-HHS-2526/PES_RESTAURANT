\# WEMOS RFID



\## Beschrijving



Deze repository bevat de code voor de Wemos RFID-module binnen het PES restaurantproject.  

De module gebruikt een Wemos D1 Mini met een MFRC522 RFID-lezer om RFID-kaarten of tags van personeel te herkennen.



Wanneer een bekende RFID-chip wordt gelezen, stuurt de Wemos via WiFi/TCP het bericht `RFID:1` naar Raspberry Pi-A. Raspberry Pi-A stuurt deze melding vervolgens door naar Raspberry Pi-B. Raspberry Pi-B communiceert daarna met de STM32-module die een servomotor aanstuurt om de personeelsdeur te openen.



\## Werking



De Wemos D1 Mini initialiseert bij het opstarten de RFID-lezer en maakt verbinding met het WiFi-netwerk.



Tijdens de loop controleert de Wemos continu of er een nieuwe RFID-kaart of tag aanwezig is. Als een kaart wordt gevonden, wordt de UID uitgelezen en omgerekend naar een chip-ID.



Wanneer de chip-ID overeenkomt met de ingestelde waarde, wordt het volgende bericht verstuurd:

```text

RFID:1

```



Dit bericht geeft aan dat een geldige RFID-chip is herkend.

