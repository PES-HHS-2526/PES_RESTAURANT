/*!
 * @file main.cpp
 * @brief Hoofdprogramma van Raspberry Pi-B voor TCP- en CANBUS-communicatie.
 *
 * Dit bestand vormt de centrale start van Raspberry Pi-B binnen het PES
 * restaurantproject. Raspberry Pi-B heeft twee hoofdtaken:
 *
 * 1. TCP-communicatie met Raspberry Pi-A via de TcpSocketRasberry-klasse.
 * 2. CANBUS-communicatie met STM32-microcontrollers via de CanSocket-klasse.
 *
 * Raspberry Pi-B ontvangt via TCP berichten van Raspberry Pi-A en stuurt deze
 * via CANBUS door naar STM32-modules. Daarnaast luistert Raspberry Pi-B naar
 * CANBUS-berichten van STM32-modules. Wanneer een STM32 bijvoorbeeld meldt dat
 * een tafeloproep is gereset, stuurt Raspberry Pi-B een bericht terug naar
 * Raspberry Pi-A.
 *
 * De TCP-server en de CAN-listener draaien in aparte threads, zodat beide
 * communicatierichtingen tegelijk actief blijven.
 *
 * @author Arbër Deda
 * @author Quinten van Ewijk
 * @date 2026
 */

#include <iostream>
#include <cstring>
#include <cerrno>       
#include <sys/socket.h> 
#include <linux/can.h>
#include <unistd.h>
#include "CanSocket.h"
#include "tcpsocketrasberry.h"
#include <thread>

/*!
 * @brief Luistert continu naar CANBUS-berichten van STM32-modules.
 *
 * Deze functie draait in een aparte thread en leest continu CAN-frames via het
 * meegegeven CanSocket-object. Wanneer een CAN-frame ontvangen wordt, worden het
 * CAN-ID en de databytes weergegeven in de terminal.
 *
 * Binnen het PES restaurantproject wordt deze functie gebruikt om resetberichten
 * van STM32-modules te ontvangen. Wanneer bijvoorbeeld de STM32-keukenmodule
 * meldt dat een tafeloproep is afgehandeld, stuurt Raspberry Pi-B via de
 * TcpSocketRasberry-server een bericht terug naar Raspberry Pi-A.
 *
 * De belangrijkste ontvangen berichten zijn:
 * - CAN-ID 0x300 met data 0x00: tafel 1 is gereset, stuur "tafel1:0" naar Pi-A.
 * - CAN-ID 0x301 met data 0x00: tafel 2 is gereset, stuur "tafel2:0" naar Pi-A.
 *
 * Het ontvangen bericht met CAN-ID 0x696 en data 0x44 wordt gebruikt als extra
 * testmelding voor een knopdruk.
 *
 * @param myCan Pointer naar het CanSocket-object waarmee CAN-frames gelezen worden.
 * @param server Pointer naar de TcpSocketRasberry-server waarmee berichten naar Pi-A gestuurd worden.
 */
void canListener(CanSocket* myCan, TcpSocketRasberry* server) {
    struct can_frame frame;
    std::cout << "--- CAN Listener Thread gestart ---" << std::endl;
    
    while (true) {
        if (myCan->receive(frame) > 0) {
            std::printf("[CAN ONTVANGEN] ID: 0x%X | Data: ", frame.can_id);
            for (int i = 0; i < frame.can_dlc; i++) {
                std::printf("%02X ", frame.data[i]);
            }
            std::printf("\n");

            if (frame.can_id == 0x300 && frame.can_dlc > 0 && frame.data[0] == 0x00) {
                std::cout << "--- 0x300 ontvangen van STM32, stuur 'tafel1:0' naar Pi-A ---" << std::endl;
                server->sendToClient("tafel1:0");
            }

            if (frame.can_id == 0x301 && frame.can_dlc > 0 && frame.data[0] == 0x00) {
                std::cout << "--- 0x301 ontvangen van STM32, stuur 'tafel2:0' naar Pi-A ---" << std::endl;
                server->sendToClient("tafel2:0");
            }
            if (frame.can_id == 0x696 && frame.can_dlc > 0 && frame.data[0] == 0x44) {
                std::cout << "--- KNOP 1 IS INGEDRUKT! ---" << std::endl;
            }
        }
    
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

/*!
 * @brief Startpunt van het Raspberry Pi-B programma.
 *
 * Deze functie initialiseert de CANBUS-verbinding op interface "can0".
 * Wanneer het openen van de CAN-interface mislukt, stopt het programma met
 * foutcode 1.
 *
 * Daarna wordt een TCP-server op poort 8080 aangemaakt. Deze server wordt in
 * een aparte thread gestart, zodat Raspberry Pi-A verbinding kan maken en
 * berichten kan sturen naar Raspberry Pi-B.
 *
 * Vervolgens wordt de CAN-listener in een tweede thread gestart. Hierdoor kan
 * Raspberry Pi-B tegelijk TCP-berichten van Pi-A ontvangen en CANBUS-berichten
 * van STM32-modules uitlezen.
 *
 * De join()-aanroepen zorgen ervoor dat het hoofdprogramma blijft wachten zolang
 * de TCP-thread en CAN-thread actief zijn.
 *
 * @return 0 wanneer het programma normaal eindigt, of 1 wanneer de CAN-interface niet geopend kan worden.
 */
int main() {
    const char* interface = "can0";
    CanSocket myCan;

    if (!myCan.open(interface)) {
        std::cerr << "CAN openen mislukt!" << std::endl;
        return 1;
    }

    TcpSocketRasberry mijnServer(8080);
    std::thread tcpThread([&mijnServer]() {
        if (mijnServer.start()) {
            mijnServer.run();
        }
    });

    std::thread canThread(canListener, &myCan, &mijnServer);

    tcpThread.join();
    canThread.join();

    return 0;
}