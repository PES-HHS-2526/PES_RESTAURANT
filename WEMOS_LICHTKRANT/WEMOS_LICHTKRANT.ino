/*!
 * @file WEMOS_LICHTKRANT.ino
 * @brief Hoofdprogramma voor de Wemos-lichtkrant binnen het PES restaurantproject.
 *
 * Dit bestand bevat de hoofdlogica voor de Wemos D1 Mini die een MAX7219
 * 32x8 LED-matrix aanstuurt. De Wemos maakt verbinding met het WiFi-netwerk
 * en wacht op berichten van Raspberry Pi-A.
 *
 * Binnenkomende berichten worden verwerkt als commando voor de lichtkrant.
 * Een normaal tekstbericht wordt direct weergegeven op de LED-matrix. Het
 * commando "SPEED:" past de scrollsnelheid aan en het commando "CLEAR" wist
 * de lichtkrant.
 *
 * De lichtkrant wordt gebruikt om informatie of meldingen zichtbaar te maken
 * binnen het restaurantgedeelte van het PES-project.
 *
 * @author Haki Abdulovski
 * @date 2026
 */

#include "WifiServerManager.h"
#include "Lichtkrant.h"

/*!
 * @brief Type MAX7219-hardware dat gebruikt wordt door de LED-matrix.
 *
 * Deze definitie geeft aan welk type MAX7219-module gebruikt wordt.
 * FC16_HW wordt vaak gebruikt bij 8x8 en 32x8 MAX7219 LED-matrix modules.
 */
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

/*!
 * @brief Aantal gekoppelde MAX7219-modules.
 *
 * De lichtkrant bestaat uit 4 gekoppelde 8x8 LED-matrix modules.
 * Samen vormen deze een 32x8 LED-matrix.
 */
#define MAX_DEVICES   4

/*!
 * @brief Chip-select pin voor de SPI-communicatie met de MAX7219.
 *
 * Deze pin wordt gebruikt om de MAX7219 LED-matrix via SPI aan te sturen.
 */
#define CS_PIN        D8

/*!
 * @brief Object voor WiFi- en socketcommunicatie met Raspberry Pi-A.
 *
 * Dit object beheert de WiFi-verbinding en TCP-server op de Wemos D1 Mini.
 * Raspberry Pi-A kan als client verbinden en tekstcommando's naar de
 * lichtkrant sturen.
 */
WifiServerManager wifi("RasberrypiWifi", "raspberry1#");

/*!
 * @brief Object voor het aansturen van de MAX7219 LED-lichtkrant.
 *
 * Dit object beheert de LED-matrix, waaronder initialisatie, tekstweergave,
 * scrollsnelheid en het uitzetten van de display.
 */
Lichtkrant lichtkrant(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

/*!
 * @brief Verwerkt een ontvangen tekstbericht of commando.
 *
 * Deze functie vertaalt een ontvangen bericht van Raspberry Pi-A naar een
 * actie op de lichtkrant.
 *
 * @param bericht Het ontvangen bericht of commando.
 */
void verwerkBericht(String bericht);

/*!
 * @brief Initialiseert de seriële communicatie, WiFi-server en lichtkrant.
 *
 * Deze functie wordt één keer uitgevoerd wanneer de Wemos D1 Mini opstart.
 * De seriële monitor wordt gestart voor debug-informatie. Daarna wordt de
 * WiFi-server gestart en wordt de lichtkrant geïnitialiseerd.
 */
void setup()
{
    Serial.begin(115200);

    wifi.begin();
    lichtkrant.begin();
}

/*!
 * @brief Hoofdlus van het programma.
 *
 * Deze functie wordt continu herhaald zolang de Wemos D1 Mini actief is.
 * Eerst wordt gecontroleerd of er een client verbonden is. Daarna wordt een
 * eventueel ontvangen bericht gelezen en verwerkt.
 *
 * Aan het einde van de loop wordt de lichtkrant bijgewerkt, zodat de tekst
 * blijft scrollen over de LED-matrix.
 */
void loop()
{
    wifi.checkClient();

    String bericht = wifi.readMessage();

    if (bericht.length() > 0)
    {
        verwerkBericht(bericht);
    }

    lichtkrant.update();
}

/*!
 * @brief Vertaalt een binnenkomend commando naar een actie op de lichtkrant.
 *
 * Als het bericht begint met "SPEED:", wordt het getal achter dit commando
 * gebruikt als nieuwe scrollsnelheid.
 *
 * Als het bericht gelijk is aan "CLEAR", wordt de lichtkrant leeg gemaakt en
 * uitgezet.
 *
 * Alle andere berichten worden behandeld als tekst die op de lichtkrant moet
 * worden weergegeven.
 *
 * @param bericht Het ontvangen bericht van Raspberry Pi-A.
 */
void verwerkBericht(String bericht)
{
    if (bericht.startsWith("SPEED:"))
    {
        int snelheid = bericht.substring(6).toInt();
        lichtkrant.setSnelheid(snelheid);
    }
    else if (bericht == "CLEAR")
    {
        lichtkrant.uitzetten();
    }
    else
    {
        lichtkrant.setTekst(bericht);
    }
}