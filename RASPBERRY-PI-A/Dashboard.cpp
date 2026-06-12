/*!
 * @file Dashboard.cpp
 * @brief Implementatie van het terminaldashboard op Raspberry Pi-A.
 *
 * Dit bestand bevat de implementatie van de Dashboard-klasse. Het dashboard
 * wordt gebruikt om apparaten binnen het PES restaurantproject via de terminal
 * te bekijken en hun netwerkstatus te controleren.
 *
 * De gebruiker kan commando's invoeren om alle apparaten te tonen, een overzicht
 * met online/offline-status te bekijken of één specifiek apparaat op te vragen.
 *
 * @author Haki Abdulovski
 * @date 2026
 */

#include "Dashboard.h"

#include <iostream>

/*!
 * @brief Constructor van de Dashboard-klasse.
 *
 * Deze constructor slaat de meegegeven lijst met apparaten op. Deze apparaten
 * worden later gebruikt om namen, IP-adressen en online statussen weer te geven.
 *
 * @param d Lijst met Device-objecten die in het dashboard beschikbaar zijn.
 */
Dashboard::Dashboard(const std::vector<Device>& d)
{
    devices = d;
}

/*!
 * @brief Start de interactieve terminalinterface van het dashboard.
 *
 * Deze functie toont de titel van het dashboard en wacht daarna continu op
 * gebruikersinvoer. Afhankelijk van het ingevoerde commando wordt de juiste
 * dashboardfunctie aangeroepen.
 *
 * Ondersteunde commando's zijn:
 * - help;
 * - devices;
 * - overzicht;
 * - device <naam>;
 * - exit.
 */
void Dashboard::start()
{
    std::string input;

    std::cout << "=================================\n";

    std::cout << " Restaurant Dashboard\n";

    std::cout << "=================================\n";

    std::cout << "Type 'help' voor commands\n";

    while(true)
    {
        std::cout << "\n> ";

        std::getline(std::cin, input);

        if(input == "exit")
        {
            break;
        }

        else if(input == "help")
        {
            showHelp();
        }

        else if(input == "devices")
        {
            showDevices();
        }

        else if(input == "overzicht")
        {
            showOverview();
        }

        else if(input.rfind("device ", 0) == 0)
        {
            std::string naam = input.substr(7);

            showSingleDevice(naam);
        }

        else
        {
            std::cout << "Unknown command\n";
        }
    }
}

/*!
 * @brief Toont alle beschikbare dashboardcommando's.
 *
 * Deze functie print een overzicht van de commando's die de gebruiker in de
 * terminal kan invoeren.
 */
void Dashboard::showHelp()
{
    std::cout << "\nCommands:\n";

    std::cout << "help\n";

    std::cout << "devices\n";

    std::cout << "overzicht\n";

    std::cout << "device <naam>\n";

    std::cout << "exit\n";
}

/*!
 * @brief Toont alle geregistreerde apparaten.
 *
 * Deze functie print de namen van alle apparaten die in het dashboard zijn
 * toegevoegd. Hierbij wordt nog geen online/offline-status gecontroleerd.
 */
void Dashboard::showDevices()
{
    std::cout << "\nDevices:\n";

    for(Device& d : devices)
    {
        std::cout << "- "
                  << d.getName()
                  << "\n";
    }
}

/*!
 * @brief Toont een statusoverzicht van alle apparaten.
 *
 * Deze functie loopt door alle geregistreerde apparaten heen en toont per
 * apparaat de naam, het IP-adres en de actuele netwerkstatus.
 *
 * De online status wordt bepaald via de online()-functie van de Device-klasse.
 */
void Dashboard::showOverview()
{
    std::cout << "\n=============================\n";

    std::cout << " OVERZICHT\n";

    std::cout << "=============================\n";

    for(Device& d : devices)
    {
        std::cout << d.getName()
                  << " ("
                  << d.getIp()
                  << ") : ";

        if(d.online())
        {
            std::cout << "ONLINE";
        }

        else
        {
            std::cout << "OFFLINE";
        }

        std::cout << "\n";
    }
}

/*!
 * @brief Toont de gegevens van één specifiek apparaat.
 *
 * Deze functie zoekt een apparaat op basis van de opgegeven naam. Wanneer het
 * apparaat gevonden wordt, worden de naam, het IP-adres en de online status
 * weergegeven.
 *
 * Als het apparaat niet gevonden wordt, wordt een foutmelding getoond.
 *
 * @param naam Naam van het apparaat dat opgezocht moet worden.
 */
void Dashboard::showSingleDevice(const std::string& naam)
{
    bool gevonden = false;

    for(Device& d : devices)
    {
        if(d.getName() == naam)
        {
            gevonden = true;

            std::cout << "\n=====================\n";

            std::cout << d.getName()
                      << "\n";

            std::cout << "=====================\n";

            std::cout << "IP: "
                      << d.getIp()
                      << "\n";

            std::cout << "Status: ";

            if(d.online())
            {
                std::cout << "ONLINE\n";
            }

            else
            {
                std::cout << "OFFLINE\n";
            }
        }
    }

    if(!gevonden)
    {
        std::cout << "Device niet gevonden\n";
    }
}