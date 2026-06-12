# Raspberry Pi-B

## Beschrijving

Deze repository bevat de code voor Raspberry Pi-B binnen het PES restaurantproject. Raspberry Pi-B vormt de brug tussen Raspberry Pi-A en de STM32-microcontrollers.

Raspberry Pi-B ontvangt berichten van Raspberry Pi-A via een TCP-socketverbinding. Deze berichten worden daarna omgezet naar CANBUS-berichten en doorgestuurd naar de STM32-modules.

Ook ontvangt Raspberry Pi-B CANBUS-berichten van STM32-modules. Bijvoorbeeld wanneer een tafeloproep in de keuken wordt gereset. Raspberry Pi-B stuurt dan een bericht terug naar Raspberry Pi-A.

## Werking

Raspberry Pi-B start een TCP-server op poort `8080`. Raspberry Pi-A maakt verbinding met deze server en stuurt berichten door van de Wemos-modules.

De CANBUS-communicatie loopt via interface `can0`.

## Communicatieketen

```text
Wemos-modules → Raspberry Pi-A → Raspberry Pi-B → STM32-modules
```

Terugmeldingen vanaf STM32 lopen terug via:

```text
STM32-modules → Raspberry Pi-B → Raspberry Pi-A → Wemos-modules
```

## Berichten

| Bericht van Pi-A | CAN-ID | CAN-data | Betekenis |
|---|---:|---|---|
| `tafel1:1` | `0x300` | `01` | Tafel 1 vraagt hulp |
| `tafel2:1` | `0x301` | `01` | Tafel 2 vraagt hulp |
| `co2:1` | `0x101` | `01` | Ventilatie aan |
| `co2:0` | `0x101` | `00` | Ventilatie uit |
| `RFID:1` | `0x501` | `01` | RFID toegestaan |
| `RFID:0` | `0x501` | `00` | RFID reset |

## Terugmeldingen van STM32

| CAN-ID | CAN-data | Bericht naar Pi-A | Betekenis |
|---|---|---|---|
| `0x300` | `00` | `tafel1:0` | Tafel 1 is gereset |
| `0x301` | `00` | `tafel2:0` | Tafel 2 is gereset |

## Bestanden

| Bestand | Beschrijving |
|---|---|
| `main.cpp` | Start de TCP-server en CAN-listener. |
| `CanSocket.h` | Headerbestand voor CANBUS-communicatie. |
| `CanSocket.cpp` | Implementatie van CANBUS openen, verzenden en ontvangen. |
| `tcpsocketrasberry.h` | Headerbestand voor de TCP-server. |
| `tcpsocketrasberry.cpp` | Implementatie van TCP-communicatie en berichtverwerking. |

## Programma starten

De code wordt op Raspberry Pi-B uitgevoerd vanuit de projectmap:

```text
pi@RPI-B:~/Documents/can_socket_3
```

Start het programma met:

```bash
./hallo
```

## Doxygen-documentatie

De code is voorzien van Doxygen-commentaar. Hiermee kan documentatie worden gegenereerd van de belangrijkste bestanden, klassen en functies.