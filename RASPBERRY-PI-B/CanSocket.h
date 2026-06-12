/*!
 * @file CanSocket.h
 * @brief Declaratie van de CanSocket-klasse voor CANBUS-communicatie op Raspberry Pi-B.
 *
 * Dit headerbestand bevat de klasse die verantwoordelijk is voor het openen,
 * sluiten, verzenden en ontvangen van CANBUS-berichten via een Linux CAN-interface,
 * bijvoorbeeld "can0".
 *
 * Binnen het PES restaurantproject wordt deze klasse gebruikt door Raspberry Pi-B
 * om te communiceren met STM32-microcontrollers via CANBUS. Raspberry Pi-B ontvangt
 * berichten van Raspberry Pi-A via TCP-sockets en zet deze om naar CAN-berichten
 * richting de STM32-modules. Andersom kan Raspberry Pi-B ook CAN-berichten van
 * STM32 ontvangen en deze via TCP terugsturen naar Raspberry Pi-A.
 *
 * @author Arbër Deda
 * @author Quinten van Ewijk
 * @date 2026
 */

#ifndef CAN_SOCKET_H
#define CAN_SOCKET_H

#include <linux/can.h>
#include <net/if.h>
#include <string>
#include <cstdint>

/*!
 * @class CanSocket
 * @brief Klasse voor het beheren van een Linux CAN-socket.
 *
 * De CanSocket-klasse vormt een eenvoudige wrapper om een Linux CAN-interface,
 * zoals "can0", te gebruiken vanuit C++.
 *
 * De klasse kan:
 * - een CAN-socket openen;
 * - de socket koppelen aan een CAN-interface;
 * - CAN-frames verzenden;
 * - CAN-frames ontvangen;
 * - de socket weer sluiten.
 *
 * Binnen Raspberry Pi-B wordt deze klasse gebruikt om CANBUS-communicatie met
 * STM32-microcontrollers mogelijk te maken. Hierdoor kunnen berichten vanuit
 * het TCP-socketnetwerk worden doorgestuurd naar de embedded STM32-modules.
 */
class CanSocket {
public:
    /*!
     * @brief Constructor van de CanSocket-klasse.
     *
     * Deze constructor maakt een CanSocket-object aan. De daadwerkelijke
     * verbinding met een CAN-interface wordt pas geopend wanneer open() wordt
     * aangeroepen.
     */
    CanSocket();

    /*!
     * @brief Destructor van de CanSocket-klasse.
     *
     * Deze destructor wordt aangeroepen wanneer het CanSocket-object wordt
     * vernietigd. De destructor wordt gebruikt om resources netjes vrij te geven,
     * zoals een geopende socketverbinding.
     */
    ~CanSocket();

    /*!
     * @brief Opent de CANBUS-verbinding via een opgegeven interface.
     *
     * Deze functie opent een Linux CAN-socket en koppelt deze aan de meegegeven
     * CAN-interface, bijvoorbeeld "can0".
     *
     * @param interface Naam van de CAN-interface die gebruikt moet worden.
     * @return true als de CAN-socket succesvol geopend en gekoppeld is, anders false.
     */
    bool open(const std::string& interface);
    
    /*!
     * @brief Sluit de CANBUS-verbinding handmatig.
     *
     * Deze functie sluit de geopende CAN-socket. Dit kan gebruikt worden om de
     * verbinding netjes af te sluiten wanneer Raspberry Pi-B stopt of wanneer
     * de CAN-interface niet meer nodig is.
     */
    void close();
    
    /*!
     * @brief Verstuurt een CAN-frame via de geopende CAN-interface.
     *
     * Deze functie maakt een CAN-frame met het opgegeven CAN-ID en de opgegeven
     * data. Daarna wordt het frame via de CAN-socket verstuurd.
     *
     * Binnen het PES restaurantproject wordt dit gebruikt om commando's naar
     * STM32-modules te sturen, bijvoorbeeld voor tafeloproepen, CO2-ventilatie
     * of RFID-deuraansturing.
     *
     * @param id Het CAN-ID waarmee het bericht verstuurd moet worden.
     * @param data Pointer naar de data die in het CAN-frame geplaatst wordt.
     * @param len Aantal databytes dat verstuurd moet worden.
     * @return Het aantal geschreven bytes bij succes, of -1 bij een fout.
     */
    int send(uint32_t id, const uint8_t* data, uint8_t len);
    
    /*!
     * @brief Ontvangt een CAN-frame via de geopende CAN-interface.
     *
     * Deze functie leest een CAN-frame van de CAN-socket en plaatst de ontvangen
     * gegevens in het meegegeven frame-object.
     *
     * Binnen Raspberry Pi-B wordt dit gebruikt om berichten van STM32-modules
     * te ontvangen, bijvoorbeeld wanneer een tafeloproep in de keuken wordt
     * gereset.
     *
     * @param frame Referentie naar een can_frame waarin het ontvangen frame wordt opgeslagen.
     * @return Het aantal gelezen bytes bij succes, of -1 bij een fout.
     */
    int receive(struct can_frame& frame);

private:
    /*!
     * @brief File descriptor van de geopende CAN-socket.
     *
     * Deze waarde verwijst naar de Linux-socket die gebruikt wordt voor
     * CANBUS-communicatie.
     */
    int _socket_fd;

    /*!
     * @brief Adresstructuur voor de CAN-socket.
     *
     * Deze structuur bevat informatie over de CAN-interface waaraan de socket
     * gekoppeld wordt.
     */
    struct sockaddr_can _addr;

    /*!
     * @brief Interface request-structuur voor het ophalen van interfacegegevens.
     *
     * Deze structuur wordt gebruikt om de interface-index van de opgegeven
     * CAN-interface, zoals "can0", op te vragen.
     */
    struct ifreq _ifr;
};

#endif