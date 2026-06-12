# STM32_GIMBAL_BOVEN

## Beschrijving

Deze repository bevat de STM32-code voor de GIMBAL_BOVEN-module binnen het PES restaurantproject. Deze STM32 wordt gebruikt als keuken-/serveersterunit voor klantoproepen van tafel 1 en tafel 2.

Wanneer een klant op de Wemos-knop van tafel 1 of tafel 2 drukt, ontvangt deze STM32 via CANBUS een bericht. Daarna gaat de bijbehorende LED in de keuken aan, zodat de medewerker of serveerster ziet dat een klant hulp nodig heeft of wil bestellen.

Wanneer de medewerker of serveerster op de resetknop van de juiste tafel drukt, gaat de keuken-LED uit en wordt via CANBUS een resetbericht teruggestuurd. Dit bericht wordt via Raspberry Pi-B en Raspberry Pi-A doorgestuurd naar de Wemos bij de klanttafel, zodat ook de LED bij de klanttafel uitgaat.

## Functie binnen het systeem

De GIMBAL_BOVEN-module verwerkt klantoproepen voor twee tafels:

- **Tafel 1:** klantoproep via Wemos, keuken-LED op de STM32 en resetknop voor de serveerster.
- **Tafel 2:** klantoproep via Wemos, keuken-LED op de STM32 en resetknop voor de serveerster.

De module vormt daarmee de fysieke keukeninterface voor het klantoproepsysteem.

## Communicatie

De STM32 communiceert met Raspberry Pi-B via CANBUS.

| CAN-ID | Data | Betekenis |
|---|---|---|
| `0x300` | `0x01` | Tafel 1 vraagt hulp, LED0 in de keuken gaat aan. |
| `0x300` | `0x00` | Tafel 1 wordt gereset, LED0 in de keuken gaat uit. |
| `0x301` | `0x01` | Tafel 2 vraagt hulp, LED1 in de keuken gaat aan. |
| `0x301` | `0x00` | Tafel 2 wordt gereset, LED1 in de keuken gaat uit. |

## Hardware

| Component | Functie |
|---|---|
| STM32 L432KC | Microcontroller voor klantoproepverwerking en CANBUS-communicatie. |
| CANBUS-module | Communicatie met Raspberry Pi-B. |
| LED0 | Keuken-LED voor tafel 1. |
| LED1 | Keuken-LED voor tafel 2. |
| Knop 0 | Resetknop voor tafel 1. |
| Knop 1 | Resetknop voor tafel 2. |
| UART | Debugcommunicatie tijdens het testen. |

## Pinfunctionaliteit

| Pin | Functie |
|---|---|
| `PA8` | LED0 voor tafel 1. |
| `PA10` | LED1 voor tafel 2. |
| `PB1` | Knop 0 voor reset van tafel 1. |
| `PA9` | Knop 1 voor reset van tafel 2. |

## Belangrijkste functies

| Functie | Beschrijving |
|---|---|
| `uartPrint()` | Stuurt debugtekst via UART naar de seriële monitor. |
| `CAN_Send()` | Verstuurt een CANBUS-bericht met één data-byte. |
| `HAL_CAN_RxFifo0MsgPendingCallback()` | Verwerkt ontvangen CANBUS-berichten voor tafel 1 en tafel 2. |

## Werking

Bij het opstarten initialiseert de STM32 de benodigde peripherals, waaronder GPIO, UART en CANBUS. Daarna worden de keuken-LED’s van tafel 1 en tafel 2 uitgezet.

Vervolgens wordt de CANBUS gestart en wordt een interrupt geactiveerd voor ontvangen CANBUS-berichten.

In de hoofdloop leest de STM32 continu de twee resetknoppen uit:

- Wanneer knop 0 wordt ingedrukt, wordt LED0 uitgezet en wordt CAN-ID `0x300` met data `0x00` verzonden.
- Wanneer knop 1 wordt ingedrukt, wordt LED1 uitgezet en wordt CAN-ID `0x301` met data `0x00` verzonden.

De resetberichten zorgen ervoor dat de oproep ook bij de Wemos-klanttafel wordt gereset.

## CANBUS callback

De functie `HAL_CAN_RxFifo0MsgPendingCallback()` wordt aangeroepen wanneer er een CANBUS-bericht binnenkomt.

Wanneer CAN-ID `0x300` wordt ontvangen, wordt tafel 1 verwerkt:

- data `0x01`: LED0 gaat aan;
- data `0x00`: LED0 gaat uit.

Wanneer CAN-ID `0x301` wordt ontvangen, wordt tafel 2 verwerkt:

- data `0x01`: LED1 gaat aan;
- data `0x00`: LED1 gaat uit.

## Doxygen-documentatie

De code is voorzien van Doxygen-commentaar. Hiermee kunnen de belangrijkste onderdelen van de code automatisch worden gedocumenteerd.

Gedocumenteerde onderdelen zijn onder andere:

- CANBUS verzendvariabelen;
- CANBUS ontvangstvariabelen;
- knopstatusvariabelen;
- `uartPrint()`;
- `CAN_Send()`;
- knopverwerking in de hoofdloop;
- `HAL_CAN_RxFifo0MsgPendingCallback()`.

## Auteur

Arbër Deda

## Projectcontext

Deze repository is onderdeel van het PES-project Embedded Systems van De Haagse Hogeschool. De STM32 GIMBAL_BOVEN-module werkt samen met Wemos-klantknoppen, Raspberry Pi-A, Raspberry Pi-B en CANBUS om klantoproepen zichtbaar te maken in de keuken en daarna te resetten.