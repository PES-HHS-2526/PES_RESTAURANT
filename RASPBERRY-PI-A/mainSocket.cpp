/*!
 * @file mainSocket.cpp
 * @brief Startprogramma voor de hoofdcommunicatie van Raspberry Pi-A.
 *
 * Dit bestand start de centrale communicatie tussen Raspberry Pi-A, meerdere
 * Wemos D1 Mini-modules en Raspberry Pi-B.
 *
 * Raspberry Pi-A maakt verbinding met verschillende Wemos-modules, zoals
 * tafel 1, tafel 2, RFID en CO2. Berichten die vanaf deze Wemos-modules
 * binnenkomen, worden via de PiCommunicator-klasse doorgestuurd naar
 * Raspberry Pi-B.
 *
 * Raspberry Pi-B zet deze berichten vervolgens om naar CANBUS-berichten voor
 * de STM32-microcontrollers. Terugmeldingen vanaf Raspberry Pi-B, zoals
 * resetberichten voor tafeloproepen, worden door Raspberry Pi-A weer naar de
 * juiste Wemos-module gestuurd.
 *
 * @author Haki Abdulovski
 * @date 2026
 */

#include "PiCommunicator.h"
#include <vector>

/*!
 * @brief Startpunt van het Raspberry Pi-A socketprogramma.
 *
 * In deze functie wordt een lijst met Wemos-modules aangemaakt. Elke Wemos-module
 * heeft een identificatie, IP-adres en poortnummer. Deze gegevens worden gebruikt
 * om verbinding te maken met de juiste Wemos D1 Mini.
 *
 * Daarna wordt een PiCommunicator-object aangemaakt. Dit object beheert de
 * communicatie tussen Raspberry Pi-A, de Wemos-modules en Raspberry Pi-B.
 *
 * De communicatie wordt gestart met runLoop().
 *
 * @return 0 wanneer het programma normaal eindigt.
 */
int main()
{
    std::vector<WemosDevice> wemosList = {
    {"tafel2", "145.52.127.229", 9091},
    {"rfid", "145.52.127.164", 6767},  
    {"tafel1", "145.52.127.128", 9090},
    {"co2", "145.52.127.241", 9092} 

    };

    PiCommunicator communicator(wemosList, "192.168.10.2", 8080);

    communicator.runLoop();

    return 0;
}