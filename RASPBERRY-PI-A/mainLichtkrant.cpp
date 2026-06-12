/*!
 * @file mainLichtkrant.cpp
 * @brief Terminalprogramma op Raspberry Pi-A voor het aansturen van de Wemos Lichtkrant.
 *
 * Dit bestand bevat het terminalprogramma waarmee een medewerker tekst en
 * commando's naar de Wemos Lichtkrant kan sturen. De Wemos Lichtkrant stuurt
 * vervolgens de 32x8 MAX7219 LED-matrix aan.
 *
 * De communicatie verloopt via de SocketClient-klasse. Raspberry Pi-A maakt
 * verbinding met de Wemos D1 Mini op poort 5555 en stuurt commando's zoals
 * tekstberichten, snelheidsinstellingen en clear-commando's.
 *
 * Binnen het PES restaurantproject wordt dit gebruikt om de lichtkrant dynamisch
 * aan te sturen zonder de Wemos-code opnieuw te hoeven uploaden.
 *
 * @author Haki Abdulovski
 * @date 2026
 */

#include <iostream>
#include <string>
#include "SocketClient.h"

/*!
 * @brief Startpunt van het lichtkrant-terminalprogramma.
 *
 * Deze functie toont een eenvoudige terminalinterface waarin de gebruiker
 * commando's kan invoeren voor de lichtkrant.
 *
 * Ondersteunde commando's zijn:
 * - "bericht <tekst>" om tekst naar de lichtkrant te sturen;
 * - "speed <waarde>" om de scrollsnelheid aan te passen;
 * - "clear" om het display leeg te maken;
 * - "help" om de beschikbare commando's te tonen;
 * - "exit" om het programma af te sluiten.
 *
 * Bij elk lichtkrantcommando wordt een tijdelijke TCP-verbinding gemaakt met
 * de Wemos Lichtkrant. Na het versturen van het bericht wordt de socket weer
 * gesloten.
 *
 * @return 0 wanneer het programma normaal wordt afgesloten.
 */
int main()
{
    const std::string WEMOS_IP = ZELF_WEMOS_IP_INVULLEN;
    const int WEMOS_PORT = 5555;


    std::string input;

    std::cout << "Welkom bij de lichtkrant\n";
    std::cout << "Voer 'help' in voor beschikbare commands\n";

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
            std::cout << "\nCommands:\n";
            std::cout << "bericht <tekst>\n";
            std::cout << "speed <waarde>\n";
            std::cout << "clear\n";
            std::cout << "exit\n";
        }

        else if(input.rfind("bericht ", 0) == 0)
        {
            SocketClient wemos(WEMOS_IP, WEMOS_PORT);

            if(!wemos.connectToServer())
            {
                std::cout << "Kan geen verbinding maken met Wemos\n";
                continue;
            }

            std::string tekst = input.substr(8);

            wemos.sendMessage(tekst + "\n");

            wemos.closeSocket();

            std::cout << "Bericht verstuurd\n";
        }

        else if(input.rfind("speed ", 0) == 0)
        {
            SocketClient wemos(WEMOS_IP, WEMOS_PORT);

            if(!wemos.connectToServer())
            {
                std::cout << "Kan geen verbinding maken met Wemos\n";
                continue;
            }

            std::string snelheid = input.substr(6);

            wemos.sendMessage("SPEED:" + snelheid + "\n");

            wemos.closeSocket();

            std::cout << "Snelheid aangepast\n";
        }

        else if(input == "clear")
        {
            SocketClient wemos(WEMOS_IP, WEMOS_PORT);

            if(!wemos.connectToServer())
            {
                std::cout << "Kan geen verbinding maken met Wemos\n";
                continue;
            }

            wemos.sendMessage("CLEAR\n");

            wemos.closeSocket();

            std::cout << "Display gewist\n";
        }

        else
        {
            std::cout << "Onbekend command\n";
        }
    }

    return 0;
}