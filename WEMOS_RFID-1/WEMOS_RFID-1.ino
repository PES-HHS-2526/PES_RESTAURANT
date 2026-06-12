/*!
 * @file WEMOS_RFID-1.ino
 * @brief Hoofdprogramma voor de Wemos RFID-module binnen het PES restaurantproject.
 *
 * Dit bestand bevat de Arduino-hoofdstructuur voor de Wemos D1 Mini die een
 * RFID-lezer aanstuurt. De daadwerkelijke RFID-logica is ondergebracht in de
 * Wemos-klasse.
 *
 * Tijdens het opstarten wordt de seriële communicatie gestart en wordt de
 * RFID-module geïnitialiseerd. In de oneindige loop wordt telkens gecontroleerd
 * of er een RFID-kaart of tag wordt aangeboden.
 *
 * Wanneer de juiste RFID-chip wordt herkend, stuurt de Wemos via WiFi/TCP een
 * bericht naar Raspberry Pi-A.
 *
 * @author Quinten van Ewijk
 * @date 2026
 */

#include <Arduino.h>
#include "Wemos.h"

/*!
 * @brief Object voor de RFID-functionaliteit van de Wemos.
 *
 * Dit object beheert het initialiseren en uitlezen van de RFID-lezer.
 * De functies setupRFID() en readRFID() worden vanuit setup() en loop()
 * aangeroepen.
 */
Wemos mijnWemos;

/*!
 * @brief Startfunctie van de Wemos RFID-module.
 *
 * Deze functie wordt één keer uitgevoerd wanneer de Wemos D1 Mini opstart.
 * De seriële monitor wordt gestart voor debug-informatie en daarna wordt
 * de RFID-module geïnitialiseerd via de Wemos-klasse.
 */
void setup() {
    Serial.begin(115200);
    mijnWemos.setupRFID();
    
}

/*!
 * @brief Hoofdlus van de Wemos RFID-module.
 *
 * Deze functie wordt continu herhaald zolang de Wemos actief is.
 * In elke iteratie wordt gecontroleerd of er een nieuwe RFID-kaart of tag
 * aanwezig is. Als er een geldige kaart wordt gelezen, kan een bericht via
 * WiFi/TCP worden verstuurd.
 */
void loop() {
    mijnWemos.readRFID();
}