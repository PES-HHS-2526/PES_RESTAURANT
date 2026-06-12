# STM32_GIMBAL_BENEDEN

## Beschrijving

Deze repository bevat de STM32-code voor de GIMBAL_BENEDEN-module binnen het PES restaurantproject. Deze STM32 stuurt twee servomotoren aan en communiceert via CANBUS met Raspberry Pi-B.

De module is verantwoordelijk voor:

- aansturing van de onderste servo voor de automatische restaurantdeur;
- aansturing van de bovenste servo voor de personeelsdeur;
- ontvangst van CANBUS-berichten vanaf Raspberry Pi-B;
- automatische terugkeer van de servo’s naar de middenpositie na een bepaalde tijd.

## Functie binnen het systeem

GIMBAL_BENEDEN stuurt twee servomotoren aan:

- **Onderste servo:** automatische restaurantdeur, aangestuurd door afstandsdetectie vanaf de Encoder-STM32.
- **Bovenste servo:** personeelsdeur, aangestuurd wanneer de RFID-module toegang verleent.

Wanneer de Encoder-STM32 via de ultrasoonsensor detecteert dat iemand bij de restaurantingang staat, wordt via CANBUS een bericht naar deze module gestuurd. De onderste servo opent dan de automatische restaurantdeur.

Wanneer de RFID-module toegang verleent aan personeel, wordt via CANBUS een bericht naar deze module gestuurd. De bovenste servo activeert dan de personeelsdeur.

## Communicatie

De STM32 communiceert met Raspberry Pi-B via CANBUS.

| CAN-ID | Data | Betekenis |
|---|---|---|
| `0x500` | `0x02` | Afstandsdetectie vanaf Encoder-STM32, onderste servo opent restaurantdeur. |
| `0x501` | `0x01` | RFID-toegang verleend, bovenste servo activeert personeelsdeur. |
| `0x501` | `0x00` | RFID-reset of toegang beëindigd, bovenste servo terug naar middenpositie. |

## Hardware

| Component | Functie |
|---|---|
| STM32 L432KC | Microcontroller voor servosturing en CANBUS-communicatie. |
| CANBUS-module | Communicatie met Raspberry Pi-B. |
| Servomotor onder | Opent/sluit de automatische restaurantdeur. |
| Servomotor boven | Stuurt de personeelsdeur aan bij geldige RFID-toegang. |
| Timer/PWM | Genereert PWM-signalen voor de servomotoren. |
| UART | Debugcommunicatie tijdens testen. |

## Belangrijkste variabelen

| Variabele | Beschrijving |
|---|---|
| `SERVO_RECHTS` | PWM-waarde voor de rechter servo-positie. |
| `SERVO_MIDDEN` | PWM-waarde voor de middenpositie van de servo. |
| `SERVO_LINKS` | PWM-waarde voor de linker servo-positie. |
| `RxHeader` | Headerstructuur voor ontvangen CANBUS-berichten. |
| `RxData` | Databuffer voor ontvangen CANBUS-berichten. |
| `deurOpen` | Statusvariabele voor de onderste servo/deur. |
| `bovenServoLinks` | Statusvariabele voor de bovenste servo. |
| `laatsteOpenTijd` | Tijdstip waarop de onderste servo/deur is geopend. |
| `laatsteDetectieBoven` | Tijdstip waarop de bovenste servo is geactiveerd. |

## Werking

Bij het opstarten initialiseert de STM32 de benodigde peripherals, waaronder GPIO, UART, timers, PWM en CANBUS. Daarna worden beide servomotoren in de middenpositie gezet.

De STM32 gebruikt twee PWM-kanalen:

- `TIM16_CHANNEL_1` voor de onderste servo van de automatische restaurantdeur;
- `TIM2_CHANNEL_2` voor de bovenste servo van de personeelsdeur.

Daarna wordt de CANBUS gestart en wordt een interrupt geactiveerd voor ontvangen CAN-berichten.

In de hoofdloop worden de statusvariabelen `deurOpen` en `bovenServoLinks` gecontroleerd. Op basis van deze variabelen worden de servomotoren naar de actieve positie of terug naar de middenpositie gestuurd.

Na 5 seconden worden de servo’s automatisch teruggezet:

- de onderste servo sluit de restaurantdeur;
- de bovenste servo gaat terug naar de middenpositie.

## CANBUS callback

De functie `HAL_CAN_RxFifo0MsgPendingCallback()` verwerkt ontvangen CANBUS-berichten.

Wanneer CAN-ID `0x500` met data `0x02` wordt ontvangen, wordt `deurOpen` op `1` gezet. Hierdoor opent de onderste servo de automatische restaurantdeur.

Wanneer CAN-ID `0x501` met data `0x01` wordt ontvangen, wordt `bovenServoLinks` op `1` gezet. Hierdoor wordt de bovenste servo geactiveerd voor de personeelsdeur.

Wanneer CAN-ID `0x501` met data `0x00` wordt ontvangen, wordt `bovenServoLinks` teruggezet naar `0`.

## Doxygen-documentatie

De code is voorzien van Doxygen-commentaar. Hiermee kunnen de belangrijkste onderdelen van de code automatisch worden gedocumenteerd.

Gedocumenteerde onderdelen zijn onder andere:

- servo-PWM waardes;
- CANBUS ontvangstvariabelen;
- statusvariabelen voor de twee servo’s;
- CAN-filterconfiguratie;
- servo-aansturing in de hoofdloop;
- `HAL_CAN_RxFifo0MsgPendingCallback()`.

## Auteur

Arbër Deda

## Projectcontext

Deze repository is onderdeel van het PES-project Embedded Systems van De Haagse Hogeschool. De STM32 GIMBAL_BENEDEN-module werkt samen met Raspberry Pi-B, de Encoder-STM32 en de RFID-functionaliteit om automatische deursturing binnen het restaurantsysteem mogelijk te maken.