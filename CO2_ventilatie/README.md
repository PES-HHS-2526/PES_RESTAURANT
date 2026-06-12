# CO2 Ventilatie

## Beschrijving

Deze repository bevat de code voor de CO2-ventilatiemodule binnen het PES restaurantproject.  
De module gebruikt een Wemos D1 Mini met een SGP30-luchtkwaliteitssensor om de eCO2-waarde in het restaurant/keukengedeelte te meten.

Wanneer de gemeten eCO2-waarde te hoog wordt, stuurt de Wemos via WiFi een bericht naar Raspberry Pi-A. Raspberry Pi-A kan dit bericht vervolgens verder verwerken binnen het restaurantdomoticasysteem. In de testopstelling wordt de ventilatie gesimuleerd met een LED.

## Werking

De Wemos D1 Mini voert continu metingen uit met de SGP30-sensor.  
Bij een eCO2-waarde hoger dan `500 ppm` wordt het bericht verstuurd:

```text
co2:1
```

Dit betekent dat de ventilatie ingeschakeld moet worden.

Bij een eCO2-waarde lager dan `450 ppm` wordt het bericht verstuurd:

```text
co2:0
```

Dit betekent dat de ventilatie uitgeschakeld mag worden.