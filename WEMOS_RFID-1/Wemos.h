/*!
 * @file Wemos.h
 * @brief Declaratie van de Wemos-klasse voor RFID-functionaliteit.
 *
 * Dit headerbestand bevat de klasse die verantwoordelijk is voor het
 * initialiseren en uitlezen van de RFID-lezer op de Wemos D1 Mini.
 *
 * De klasse maakt gebruik van de MFRC522-library en SPI-communicatie om
 * RFID-kaarten of tags te lezen. Wanneer een bekende RFID-chip wordt herkend,
 * kan er via de WiFi-servermanager een bericht naar Raspberry Pi-A worden
 * gestuurd.
 *
 * @author Quinten van Ewijk
 * @date 2026
 */

#ifndef WEMOS_H
#define WEMOS_H

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

/*!
 * @brief Slave-select pin voor de RFID-lezer.
 *
 * Deze pin wordt gebruikt als SS/SDA-pin voor de SPI-communicatie met de
 * MFRC522 RFID-module. Op de Wemos D1 Mini komt dit overeen met D8.
 */
#define SS_PIN 15   // Pin: D8

/*!
 * @brief Resetpin voor de RFID-lezer.
 *
 * Deze pin wordt gebruikt om de MFRC522 RFID-module te resetten.
 * Op de Wemos D1 Mini komt dit overeen met D3.
 */
#define RST_PIN 0   // Pin: D3

/*!
 * @brief Globale variabele waarin de berekende RFID-chip-ID wordt opgeslagen.
 *
 * Deze variabele wordt gebruikt om de gelezen UID-bytes van een RFID-kaart
 * om te zetten naar één getal dat vergeleken kan worden met een bekende waarde.
 */
extern long chipID;

/*!
 * @brief Globaal MFRC522-object voor de RFID-lezer.
 *
 * Dit object verzorgt de directe communicatie met de MFRC522 RFID-module.
 */
extern MFRC522 mfrc522;

/*!
 * @class Wemos
 * @brief Klasse voor het initialiseren en uitlezen van de RFID-module.
 *
 * De Wemos-klasse bevat de functies voor het starten van de RFID-lezer en het
 * controleren of er een RFID-kaart of tag wordt aangeboden.
 *
 * Binnen het PES restaurantproject wordt deze klasse gebruikt om RFID-invoer
 * te detecteren en bij herkenning een bericht via WiFi/TCP door te sturen.
 */
class Wemos {
private:
/*!
 * @brief Interne variabele binnen de Wemos-klasse.
 *
 * Deze variabele is aanwezig als private attribuut binnen de klasse.
 */
int iets;
public:
/*!
 * @brief Initialiseert de RFID-lezer en de WiFi-communicatie.
 *
 * Deze functie start SPI, initialiseert de MFRC522 RFID-module, toont
 * versie-informatie via de seriële monitor en start de WiFi-servermanager.
 */
void setupRFID();

/*!
 * @brief Leest een RFID-kaart of tag uit.
 *
 * Deze functie controleert of er een nieuwe RFID-kaart aanwezig is. Als er een
 * kaart wordt gevonden, wordt de UID uitgelezen en omgerekend naar een chip-ID.
 * Wanneer deze chip-ID overeenkomt met de ingestelde waarde, wordt via WiFi/TCP
 * het bericht "RFID:1" verstuurd.
 */
void readRFID();
};
#endif