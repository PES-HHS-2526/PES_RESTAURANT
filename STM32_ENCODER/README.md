# STM32_ENCODER

## Beschrijving

Deze repository bevat de STM32-code voor de Encoder-module binnen het PES restaurantproject. Deze STM32 combineert meerdere functies binnen het embedded systeem en communiceert via CANBUS met Raspberry Pi-B.

De STM32 Encoder-module is verantwoordelijk voor:

- CANBUS-communicatie met Raspberry Pi-B;
- RGB-LED aansturing voor de CO2/eCO2-ventilatiestatus;
- aansturing van een MAX7219 LED-matrix via SPI;
- ultrasoonsensorfunctionaliteit via trigger/echo voor detectie bij de restaurantingang;
- knopdetectie en CAN-terugmelding.

## Functie binnen het systeem

De STM32 Encoder-module verwerkt lokale input en output binnen het restaurantsysteem. De module kan berichten ontvangen via CANBUS en stuurt op basis daarvan hardware aan. Daarnaast kan de module zelf CANBUS-berichten verzenden wanneer een knop wordt ingedrukt of wanneer de ultrasoonsensor een object of persoon detecteert.

De ultrasoonsensor wordt gebruikt voor de automatische ingangdeur van het restaurant. Wanneer de gemeten afstand kleiner is dan 5 cm, stuurt de STM32 een CANBUS-bericht naar Raspberry Pi-B.

## Communicatie

De STM32 communiceert met Raspberry Pi-B via CANBUS.

| CAN-ID | Data | Betekenis |
|---|---|---|
| `0x101` | `0x01` | CO2/eCO2-status ontvangen, RGB-LED rood. |
| `0x101` | `0x00` | CO2/eCO2-status normaal, RGB-LED groen. |
| `0x300` | `0x01` | Knop ingedrukt. |
| `0x300` | `0x00` | Knop losgelaten of reset. |
| `0x500` | `0x02` | Afstand kleiner dan 5 cm, ingangdeur openen. |

## Hardware

| Component | Functie |
|---|---|
| STM32 L432KC | Microcontroller voor verwerking en CANBUS-communicatie. |
| CANBUS-module | Communicatie met Raspberry Pi-B. |
| RGB-LED | Weergave van CO2/eCO2-ventilatiestatus. |
| MAX7219 LED-matrix | Lichtkrant / tekstweergave via SPI. |
| Ultrasoonsensor | Detectie bij de automatische restaurantingang. |
| Drukknop | Lokale knopdetectie en CAN-terugmelding. |

## Belangrijkste functies

| Functie | Beschrijving |
|---|---|
| `microDelay()` | Maakt een korte vertraging in microseconden voor de ultrasoonsensor. |
| `RGB_Uit()` | Zet alle RGB-LED kanalen uit. |
| `RGB_Rood()` | Zet de RGB-LED op rood. |
| `RGB_Groen()` | Zet de RGB-LED op groen. |
| `max7219Send()` | Verstuurt data naar de MAX7219 LED-driver via SPI. |
| `max7219Init()` | Initialiseert de MAX7219 LED-matrix. |
| `getFontIndex()` | Bepaalt de index van een karakter in de fonttabel. |
| `scrollStep()` | Voert één scrollstap uit voor tekst op de LED-matrix. |
| `HAL_CAN_RxFifo0MsgPendingCallback()` | Verwerkt ontvangen CANBUS-berichten. |

## Werking

Bij het opstarten initialiseert de STM32 alle benodigde peripherals, waaronder GPIO, UART, CANBUS, I2C, SPI en timerfunctionaliteit. Daarna worden de CAN-filterinstellingen geconfigureerd en wordt de CAN-interrupt geactiveerd.

Tijdens de hoofdloop worden de knopstatus en ultrasoonsensorstatus gecontroleerd. De lichtkrant blijft ondertussen scrollen. Wanneer de afstand kleiner is dan 5 cm, wordt een CANBUS-bericht verzonden met CAN-ID `0x500` en data `0x02`.

Wanneer de STM32 via CANBUS een bericht ontvangt met CAN-ID `0x101`, wordt de RGB-LED aangepast op basis van de ontvangen data.

## Doxygen-documentatie

De code is voorzien van Doxygen-commentaar. Hiermee kunnen de belangrijkste functies, variabelen en de werking van het bestand automatisch worden gedocumenteerd.

Gedocumenteerde onderdelen zijn onder andere:

- `microDelay()`
- `RGB_Uit()`
- `RGB_Rood()`
- `RGB_Groen()`
- `max7219Send()`
- `max7219Init()`
- `getFontIndex()`
- `scrollStep()`
- `HAL_CAN_RxFifo0MsgPendingCallback()`

## Auteur

Arbër Deda  
Haki Abdulovski

## Projectcontext

Deze repository is onderdeel van het PES-project Embedded Systems van De Haagse Hogeschool. De STM32 Encoder-module werkt samen met Raspberry Pi-B, Raspberry Pi-A, Wemos-modules en andere STM32-modules binnen het restaurantsysteem.