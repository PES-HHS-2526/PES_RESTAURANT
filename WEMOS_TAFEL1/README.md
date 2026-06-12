# WEMOS Tafel 1

## Beschrijving

Deze repository bevat de code voor de Wemos D1 Mini van tafel 1 binnen het PES restaurantproject.  
De module stelt een klantoproepsysteem voor. Wanneer een klant op de knop bij tafel 1 drukt, gaat de LED bij de tafel aan en wordt via WiFi/TCP het bericht `tafel1:1` naar Raspberry Pi-A gestuurd.

Raspberry Pi-A stuurt deze melding door naar Raspberry Pi-B. Raspberry Pi-B stuurt vervolgens via CANBUS een bericht naar een STM32-module in de keuken. Hierdoor kan in de keuken zichtbaar worden dat tafel 1 hulp nodig heeft of dat er een bestelling gekoppeld is aan tafel 1.

Wanneer de serveerster in de keuken op een resetknop drukt, stuurt de STM32 via Raspberry Pi-B en Raspberry Pi-A het bericht `tafel1:0` terug naar de Wemos. De Wemos zet daarna de LED bij de klant uit. Dit geeft aan dat de klant geholpen is of dat de bestelling klaar is.

## Werking

De Wemos D1 Mini maakt verbinding met het WiFi-netwerk en start een TCP-server. Raspberry Pi-A kan als client verbinding maken met deze server.

Wanneer de klant op de knop drukt, wordt de LED bij tafel 1 aangezet en wordt het volgende bericht verstuurd:

```text
tafel1:1
```

Dit bericht betekent dat tafel 1 hulp of aandacht nodig heeft.

Wanneer de Wemos het volgende bericht ontvangt:

```text
tafel1:0
```

wordt de LED bij tafel 1 uitgezet.

Dit bericht betekent dat de oproep van tafel 1 is afgehandeld.