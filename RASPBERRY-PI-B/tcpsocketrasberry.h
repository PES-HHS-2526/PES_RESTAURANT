/*!
 * @file tcpsocketrasberry.h
 * @brief Declaratie van de TcpSocketRasberry-klasse voor TCP-communicatie op Raspberry Pi-B.
 *
 * Dit headerbestand bevat de klasse die verantwoordelijk is voor het opzetten
 * van een TCP-server op Raspberry Pi-B. Raspberry Pi-A kan verbinding maken met
 * deze server om berichten door te sturen naar Raspberry Pi-B.
 *
 * Binnen het PES restaurantproject gebruikt Raspberry Pi-B deze klasse om
 * berichten van Raspberry Pi-A te ontvangen en deze vervolgens via de CanSocket-
 * klasse door te sturen naar STM32-microcontrollers via CANBUS.
 *
 * Daarnaast kan Raspberry Pi-B via deze klasse berichten terugsturen naar
 * Raspberry Pi-A, bijvoorbeeld wanneer een STM32-module via CANBUS meldt dat
 * een tafeloproep is gereset.
 *
 * @author Arbër Deda
 * @author Quinten van Ewijk
 * @date 2026
 */

#ifndef TCPSOCKETRASBERRY_H
#define TCPSOCKETRASBERRY_H

#include <netinet/in.h>
#include <string>
#include "CanSocket.h"
#include <mutex>

/*!
 * @class TcpSocketRasberry
 * @brief Klasse voor TCP-servercommunicatie tussen Raspberry Pi-B en Raspberry Pi-A.
 *
 * De TcpSocketRasberry-klasse start een TCP-server op Raspberry Pi-B.
 * Raspberry Pi-A maakt verbinding met deze server en stuurt tekstberichten
 * door, zoals tafeloproepen, CO2-statussen en RFID-statussen.
 *
 * De klasse verwerkt ontvangen TCP-berichten en gebruikt intern een CanSocket-
 * object om deze berichten om te zetten naar CANBUS-berichten richting de
 * STM32-microcontrollers.
 *
 * De klasse bevat ook een mutex om het versturen van berichten naar de client
 * veilig te houden wanneer meerdere delen van het programma dezelfde
 * socketverbinding gebruiken.
 */
class TcpSocketRasberry {
private:
    /*!
     * @brief File descriptor van de TCP-serversocket.
     *
     * Deze socket wordt gebruikt om inkomende verbindingen van Raspberry Pi-A
     * te accepteren.
     */
    int _server_fd;

    /*!
     * @brief Poortnummer waarop de TCP-server luistert.
     *
     * Dit poortnummer wordt via de constructor ingesteld.
     */
    int _port;

    /*!
     * @brief Adresstructuur van de TCP-server.
     *
     * Deze structuur bevat onder andere het IP-adrestype en het poortnummer
     * waarop de server luistert.
     */
    struct sockaddr_in _address;

    /*!
     * @brief Optiewaarde voor het configureren van de socket.
     *
     * Deze waarde wordt gebruikt bij socketopties, bijvoorbeeld om hergebruik
     * van het adres of de poort mogelijk te maken.
     */
    int _opt;

    /*!
     * @brief File descriptor van de actieve clientverbinding.
     *
     * Deze socket wordt gebruikt voor de communicatie met Raspberry Pi-A nadat
     * Raspberry Pi-A verbinding heeft gemaakt met de server.
     */
    int _client_socket;

    /*!
     * @brief Mutex voor het beveiligen van socketcommunicatie.
     *
     * Deze mutex voorkomt dat meerdere stukken code tegelijk naar dezelfde
     * TCP-clientsocket schrijven.
     */
    std::mutex _socketMutex;  
    
	/*!
     * @brief CANBUS-object voor communicatie met STM32-microcontrollers.
     *
     * Dit object wordt gebruikt om ontvangen TCP-berichten vanaf Raspberry Pi-A
     * door te sturen naar de STM32-modules via CANBUS.
     */
	CanSocket _canBus;
	
    /*!
     * @brief Verwerkt een ontvangen TCP-bericht van Raspberry Pi-A.
     *
     * Deze functie interpreteert het ontvangen tekstbericht en zet dit om naar
     * de juiste CANBUS-actie. Voorbeelden hiervan zijn tafeloproepen,
     * CO2-ventilatiestatussen en RFID-deurcommando's.
     *
     * @param client_socket De actieve clientverbinding met Raspberry Pi-A.
     * @param message Het ontvangen tekstbericht dat verwerkt moet worden.
     */
    void handleMessage(int client_socket, const char* message);

public:
    /*!
     * @brief Constructor van de TcpSocketRasberry-klasse.
     *
     * Deze constructor stelt het poortnummer in waarop de TCP-server moet
     * luisteren. De daadwerkelijke server wordt gestart met start().
     *
     * @param port Het poortnummer waarop Raspberry Pi-B moet luisteren.
     */
    TcpSocketRasberry(int port);

    /*!
     * @brief Destructor van de TcpSocketRasberry-klasse.
     *
     * Deze destructor wordt gebruikt om geopende sockets netjes te sluiten
     * wanneer het object wordt vernietigd.
     */
    ~TcpSocketRasberry();

    /*!
     * @brief Start de TCP-server op Raspberry Pi-B.
     *
     * Deze functie maakt de serversocket aan, configureert de socket en koppelt
     * deze aan het ingestelde poortnummer.
     *
     * @return true als de server succesvol gestart is, anders false.
     */
    bool start();

    /*!
     * @brief Draait de hoofdlogica van de TCP-server.
     *
     * Deze functie accepteert verbindingen van Raspberry Pi-A en leest
     * binnenkomende berichten. Ontvangen berichten worden doorgestuurd naar
     * handleMessage() voor verdere verwerking.
     */
    void run();

    /*!
     * @brief Stuurt een tekstbericht terug naar Raspberry Pi-A.
     *
     * Deze functie wordt gebruikt om berichten vanuit Raspberry Pi-B terug te
     * sturen naar de verbonden Raspberry Pi-A-client.
     *
     * @param message Het tekstbericht dat naar Raspberry Pi-A verstuurd moet worden.
     */
    void sendToClient(const std::string& message);
};

#endif