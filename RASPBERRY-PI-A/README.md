# Raspberry Pi-A

## Beschrijving

Deze repository bevat de code voor Raspberry Pi-A binnen het PES restaurantproject. Raspberry Pi-A vormt de centrale schakel tussen de Wemos D1 Mini-modules en Raspberry Pi-B.

Raspberry Pi-A maakt via TCP-sockets verbinding met meerdere Wemos-modules, zoals de tafeloproepen, CO2/eCO2-sensor, RFID-module en lichtkrant. Berichten van deze Wemos-modules worden verwerkt en waar nodig doorgestuurd naar Raspberry Pi-B. Raspberry Pi-B zet deze berichten vervolgens om naar CANBUS-berichten richting de STM32-microcontrollers.

Daarnaast bevat deze repository ook een terminalprogramma voor de lichtkrant en een terminalgebaseerd dashboard waarmee apparaten gecontroleerd kunnen worden.

## Onderdelen

Deze repository bestaat uit drie hoofdonderdelen:

| Onderdeel | Beschrijving |
|---|---|
| Hoofdcommunicatie | Verwerkt berichten tussen Wemos-modules, Raspberry Pi-A en Raspberry Pi-B. |
| Lichtkrant-terminal | Stuurt tekst, snelheid en clear-commando's naar de Wemos Lichtkrant. |
| Dashboard | Toont apparaten en controleert via ping of apparaten online zijn. |

## Communicatieketen

De algemene communicatieketen is:

```text
Wemos-modules → Raspberry Pi-A → Raspberry Pi-B → STM32-modules
```

Terugmeldingen vanaf STM32 lopen terug via:

```text
STM32-modules → Raspberry Pi-B → Raspberry Pi-A → Wemos-modules
```

## Hoofdcommunicatie

De hoofdcommunicatie wordt gestart vanuit `mainSocket.cpp`. Hierin wordt ingesteld met welke Wemos-modules Raspberry Pi-A verbinding maakt.

Voorbeelden van berichten die Raspberry Pi-A verwerkt:

| Bericht | Betekenis |
|---|---|
| `tafel1:1` | Tafel 1 vraagt hulp. |
| `tafel2:1` | Tafel 2 vraagt hulp. |
| `tafel1:0` | Oproep van tafel 1 wordt gereset. |
| `tafel2:0` | Oproep van tafel 2 wordt gereset. |
| `co2:1` | CO2/eCO2-waarde is te hoog. |
| `co2:0` | CO2/eCO2-waarde is weer normaal. |
| `RFID:1` | Geldige RFID-kaart is gescand. |
| `RFID:0` | RFID-reset of toegang geweigerd. |

## Lichtkrant-terminal

De lichtkrant wordt aangestuurd via `mainLichtkrant.cpp`. Dit programma gebruikt de `SocketClient`-klasse om verbinding te maken met de Wemos Lichtkrant.

Beschikbare commando's:

```text
help
bericht <tekst>
speed <waarde>
clear
exit
```

Voorbeelden:

```text
bericht Welkom in het restaurant
speed 50
clear
```

## Dashboard

Het dashboard wordt gestart vanuit `mainDashboard.cpp`. In dit programma worden apparaten toegevoegd met een naam en IP-adres.

Het dashboard gebruikt de `Device`-klasse om via een ping te controleren of een apparaat online of offline is.

Beschikbare commando's:

```text
help
devices
overzicht
device <naam>
exit
```

## Bestanden

| Bestand | Beschrijving |
|---|---|
| `mainSocket.cpp` | Startprogramma voor de hoofdcommunicatie tussen Wemos, Raspberry Pi-A en Raspberry Pi-B. |
| `PiCommunicator.h` | Headerbestand van de centrale communicatieklasse. |
| `PiCommunicator.cpp` | Implementatie van de communicatie tussen Wemos-modules en Raspberry Pi-B. |
| `SocketClient.h` | Headerbestand van de TCP-clientklasse. |
| `SocketClient.cpp` | Implementatie van TCP-clientcommunicatie. |
| `mainLichtkrant.cpp` | Terminalprogramma voor het aansturen van de Wemos Lichtkrant. |
| `mainDashboard.cpp` | Startprogramma van het dashboard. |
| `Dashboard.h` | Headerbestand van de Dashboard-klasse. |
| `Dashboard.cpp` | Implementatie van het terminaldashboard. |
| `Device.h` | Headerbestand van de Device-klasse. |
| `Device.cpp` | Implementatie voor apparaatstatuscontrole via ping. |

## Programma's compileren

Hoofdcommunicatie:

```bash
g++ mainSocket.cpp PiCommunicator.cpp SocketClient.cpp -o socketprogramma -pthread
```

Lichtkrant-terminal:

```bash
g++ mainLichtkrant.cpp SocketClient.cpp -o lichtkrant
```

Dashboard:

```bash
g++ mainDashboard.cpp Dashboard.cpp Device.cpp -o dashboard
```

## Programma's starten

Hoofdcommunicatie:

```bash
./socketprogramma
```

Lichtkrant-terminal:

```bash
./lichtkrant
```

Dashboard:

```bash
./dashboard
```

## Doxygen-documentatie

De code is voorzien van Doxygen-commentaar. Hiermee kan documentatie worden gegenereerd van de belangrijkste bestanden, klassen en functies.

Gedocumenteerde onderdelen zijn onder andere:

- `SocketClient`
- `PiCommunicator`
- `Dashboard`
- `Device`
- `mainSocket.cpp`
- `mainLichtkrant.cpp`
- `mainDashboard.cpp`

## Projectcontext

Deze repository is onderdeel van het PES-project Embedded Systems van De Haagse Hogeschool. Raspberry Pi-A wordt gebruikt als centrale communicatiehub tussen draadloze Wemos D1 Mini-modules en Raspberry Pi-B. Hierdoor kunnen klantoproepen, CO2/eCO2-statussen, RFID-berichten, lichtkrantcommando's en dashboardinformatie binnen het restaurantsysteem worden verwerkt.