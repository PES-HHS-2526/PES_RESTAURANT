/*!
 * @file SocketClient.h
 * @brief Declaratie van de SocketClient-klasse voor TCP-clientcommunicatie op Raspberry Pi-A.
 *
 * Dit headerbestand bevat de SocketClient-klasse. Deze klasse wordt gebruikt
 * door Raspberry Pi-A om verbinding te maken met andere systemen via TCP-sockets.
 *
 * Binnen het PES restaurantproject wordt deze klasse onder andere gebruikt voor:
 * - communicatie van Raspberry Pi-A naar Raspberry Pi-B;
 * - communicatie van Raspberry Pi-A naar Wemos-modules;
 * - het versturen van lichtkrantcommando's naar de Wemos Lichtkrant;
 * - het ontvangen en doorsturen van berichten vanuit de verschillende subsystemen.
 *
 * @author Haki Abdulovski
 * @date 2026
 */

#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include <string>
#include <mutex>
#include <functional>
#include <sys/types.h>

/*!
 * @class SocketClient
 * @brief Klasse voor TCP-clientverbindingen vanaf Raspberry Pi-A.
 *
 * De SocketClient-klasse beheert één TCP-clientverbinding naar een server.
 * Dit kan bijvoorbeeld Raspberry Pi-B zijn, maar ook een Wemos D1 Mini die als
 * WiFi TCP-server draait.
 *
 * De klasse kan:
 * - verbinding maken met een server;
 * - berichten versturen;
 * - berichten ontvangen;
 * - een receive-loop draaien met een callbackfunctie;
 * - de socketverbinding sluiten.
 *
 * De mutex wordt gebruikt om het versturen van berichten veilig te houden
 * wanneer meerdere delen van het programma dezelfde socketverbinding gebruiken.
 */
class SocketClient
{
public:
    /*!
     * @brief Constructor van de SocketClient-klasse.
     *
     * Deze constructor slaat het IP-adres, poortnummer en optioneel een naam op.
     * De daadwerkelijke socketverbinding wordt pas gemaakt wanneer
     * connectToServer() wordt aangeroepen.
     *
     * @param ip IP-adres van de server waarmee verbinding gemaakt moet worden.
     * @param port Poortnummer van de server.
     * @param name Naam van de verbinding of het apparaat.
     */
    SocketClient(const std::string& ip, int port, const std::string& name = "");

    /*!
     * @brief Destructor van de SocketClient-klasse.
     *
     * Deze destructor wordt gebruikt om de socketverbinding netjes te sluiten
     * wanneer het SocketClient-object wordt vernietigd.
     */
    ~SocketClient();

    /*!
     * @brief Maakt verbinding met de ingestelde server.
     *
     * Deze functie maakt een TCP-socket aan en probeert verbinding te maken met
     * het ingestelde IP-adres en poortnummer.
     *
     * @return true als de verbinding succesvol is gemaakt, anders false.
     */
    bool connectToServer();

    /*!
     * @brief Stelt een ontvangsttimeout in voor de socket.
     *
     * Met deze functie kan worden ingesteld hoelang receiveMessage() maximaal
     * wacht op een binnenkomend bericht.
     *
     * @param milliseconds Timeoutwaarde in milliseconden.
     * @return true als de timeout succesvol is ingesteld, anders false.
     */
    bool setReceiveTimeout(long milliseconds);

    /*!
     * @brief Verstuurt een tekstbericht naar de verbonden server.
     *
     * Deze functie stuurt een stringbericht via de actieve TCP-verbinding.
     *
     * @param msg Het bericht dat verstuurd moet worden.
     * @return Het aantal verzonden bytes, of -1 bij een fout.
     */
    ssize_t sendMessage(const std::string& msg);

    /*!
     * @brief Ontvangt een tekstbericht van de verbonden server.
     *
     * Deze functie probeert een bericht van de actieve TCP-verbinding te lezen
     * en plaatst de ontvangen tekst in de meegegeven string.
     *
     * @param msg Referentie naar de string waarin het ontvangen bericht wordt opgeslagen.
     * @return Het aantal ontvangen bytes, 0 bij gesloten verbinding, of -1 bij een fout.
     */
    ssize_t receiveMessage(std::string& msg);

    /*!
     * @brief Sluit de actieve socketverbinding.
     *
     * Deze functie sluit de socket wanneer er nog een actieve verbinding open is.
     */
    void closeSocket();

    /*!
     * @brief Start een receive-loop voor binnenkomende berichten.
     *
     * Deze functie blijft berichten ontvangen zolang de socketverbinding actief is.
     * Voor elk ontvangen bericht wordt de meegegeven callbackfunctie aangeroepen.
     *
     * Binnen Raspberry Pi-A wordt dit gebruikt om berichten van Wemos-modules of
     * Raspberry Pi-B continu te kunnen verwerken.
     *
     * @param onMessage Callbackfunctie die wordt uitgevoerd bij een ontvangen bericht.
     */
    void receiveLoop(std::function<void(const std::string&)> onMessage);

    /*!
     * @brief Geeft de naam van deze socketclient terug.
     *
     * De naam wordt gebruikt om een verbinding of apparaat herkenbaar te maken,
     * bijvoorbeeld "tafel1", "co2", "rfid" of "lichtkrant".
     *
     * @return Constante referentie naar de naam van deze socketclient.
     */
    const std::string& getName() const { return name; }

private:
    /*!
     * @brief IP-adres van de server waarmee verbinding wordt gemaakt.
     */
    std::string ipAddress;

    /*!
     * @brief Poortnummer van de server waarmee verbinding wordt gemaakt.
     */
    int portNumber;

    /*!
     * @brief File descriptor van de TCP-socket.
     */
    int sock;

    /*!
     * @brief Naam van de verbinding of het apparaat.
     */
    std::string name;

    /*!
     * @brief Mutex voor het beveiligen van verzendacties.
     *
     * Deze mutex voorkomt dat meerdere threads tegelijk via dezelfde socket
     * proberen te verzenden.
     */
    std::mutex sendMutex;
};

#endif