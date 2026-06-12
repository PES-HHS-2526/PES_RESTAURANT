/*!
 * @file Dashboard.h
 * @brief Declaratie van de Dashboard-klasse voor het terminaldashboard op Raspberry Pi-A.
 *
 * Dit headerbestand bevat de Dashboard-klasse. Deze klasse wordt gebruikt om
 * een terminalgebaseerd dashboard te tonen waarmee de status van apparaten
 * binnen het PES restaurantproject bekeken kan worden.
 *
 * Het dashboard gebruikt Device-objecten om apparaten op te slaan en hun online
 * status te controleren.
 *
 * @author Haki Abdulovski
 * @date 2026
 */

#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "Device.h"

#include <vector>
#include <string>

/*!
 * @class Dashboard
 * @brief Klasse voor het tonen en bedienen van het terminaldashboard.
 *
 * De Dashboard-klasse beheert een lijst met apparaten en biedt commando's om
 * deze apparaten weer te geven. Via het dashboard kan de gebruiker een overzicht
 * bekijken van alle apparaten of de status van één specifiek apparaat opvragen.
 *
 * Binnen het PES restaurantproject wordt dit gebruikt om netwerkapparaten zoals
 * Wemos-modules en Raspberry Pi-systemen te monitoren.
 */
class Dashboard
{
private:
    /*!
     * @brief Lijst met apparaten die in het dashboard worden weergegeven.
     *
     * Elk apparaat wordt opgeslagen als Device-object met een naam en IP-adres.
     */
    std::vector<Device> devices;

public:
    /*!
     * @brief Constructor van de Dashboard-klasse.
     *
     * Deze constructor ontvangt een lijst met apparaten en slaat deze op in het
     * dashboard.
     *
     * @param d Lijst met Device-objecten die in het dashboard getoond worden.
     */
    Dashboard(const std::vector<Device>& d);

    /*!
     * @brief Start het terminaldashboard.
     *
     * Deze functie start de interactieve terminalomgeving waarin de gebruiker
     * commando's kan invoeren, zoals help, devices, overzicht, device en exit.
     */
    void start();

    /*!
     * @brief Toont de beschikbare dashboardcommando's.
     *
     * Deze functie print een overzicht van de commando's die de gebruiker in het
     * dashboard kan invoeren.
     */
    void showHelp();

    /*!
     * @brief Toont de namen en IP-adressen van alle geregistreerde apparaten.
     *
     * Deze functie geeft de volledige apparatenlijst weer zonder direct de online
     * status te controleren.
     */
    void showDevices();

    /*!
     * @brief Toont een overzicht van alle apparaten met hun online status.
     *
     * Deze functie controleert per apparaat of het bereikbaar is en toont daarna
     * de status als ONLINE of OFFLINE.
     */
    void showOverview();

    /*!
     * @brief Toont de gegevens en status van één specifiek apparaat.
     *
     * Deze functie zoekt een apparaat op basis van de opgegeven naam. Als het
     * apparaat gevonden wordt, worden het IP-adres en de online status getoond.
     *
     * @param naam Naam van het apparaat waarvan de status getoond moet worden.
     */
    void showSingleDevice(const std::string& naam);
};

#endif