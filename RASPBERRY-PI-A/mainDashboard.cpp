/*!
 * @file mainDashboard.cpp
 * @brief Startprogramma voor het terminaldashboard op Raspberry Pi-A.
 *
 * Dit bestand start het dashboardprogramma op Raspberry Pi-A. In dit programma
 * wordt een lijst met apparaten aangemaakt die binnen het PES restaurantproject
 * gecontroleerd kunnen worden.
 *
 * Het dashboard gebruikt de Device-klasse om per apparaat de naam en het IP-adres
 * op te slaan. Vervolgens gebruikt de Dashboard-klasse deze lijst om apparaten
 * weer te geven en hun online/offline-status te controleren via ping.
 *
 * @author Haki Abdulovski
 * @date 2026
 */

#include "Dashboard.h"

#include <vector>

/*!
 * @brief Startpunt van het dashboardprogramma.
 *
 * In deze functie wordt een lijst met apparaten aangemaakt die in het dashboard
 * zichtbaar zijn. Voor elk apparaat wordt een naam en IP-adres ingesteld.
 *
 * Daarna wordt een Dashboard-object aangemaakt en wordt de interactieve
 * terminalinterface gestart met start().
 *
 * @return 0 wanneer het programma normaal wordt afgesloten.
 */
int main()
{
    std::vector<Device> devices;

    devices.push_back(Device("Pi-B point-to-point", "192.168.10.2"));

    devices.push_back(Device("Wemos Lichtkrant", "145.52.127.240"));

    devices.push_back(Device("Pi-B device", "145.52.127.236"));

    devices.push_back(Device("Wemos RFID", "145.52.127.164"));

    devices.push_back(Device("Tafel1", "145.52.127.128"));

    devices.push_back(Device("CO2-sensor", "145.52.127.241"));

    Dashboard dashboard(devices);

    dashboard.start();

    return 0;
}