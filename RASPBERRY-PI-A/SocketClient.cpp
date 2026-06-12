/*!
 * @file SocketClient.cpp
 * @brief Implementatie van de SocketClient-klasse voor TCP-clientcommunicatie op Raspberry Pi-A.
 *
 * Dit bestand bevat de implementatie van de SocketClient-klasse. Deze klasse
 * wordt gebruikt door Raspberry Pi-A om TCP-verbindingen te maken met andere
 * onderdelen binnen het PES restaurantproject.
 *
 * De klasse wordt onder andere gebruikt voor communicatie met Raspberry Pi-B,
 * de Wemos-modules en de Wemos Lichtkrant. Via deze verbindingen kunnen berichten
 * worden verstuurd en ontvangen, zoals tafeloproepen, CO2-statussen, RFID-statussen
 * en lichtkrantcommando's.
 *
 * @author Haki Abdulovski
 * @date 2026
 */

#include "SocketClient.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <cerrno>

/*!
 * @brief Constructor van de SocketClient-klasse.
 *
 * Deze constructor slaat het IP-adres, poortnummer en de naam van de verbinding op.
 * De socket wordt standaard op -1 gezet, zodat duidelijk is dat er nog geen actieve
 * verbinding is.
 *
 * Als er geen naam wordt meegegeven, wordt het IP-adres gebruikt als herkenbare naam
 * voor de verbinding.
 *
 * @param ip IP-adres van de server waarmee verbinding gemaakt moet worden.
 * @param port Poortnummer van de server.
 * @param name Naam van de verbinding of het apparaat.
 */
SocketClient::SocketClient(const std::string& ip, int port, const std::string& name)
    : ipAddress(ip), portNumber(port), sock(-1),
      name(name.empty() ? ip : name) {}

/*!
 * @brief Destructor van de SocketClient-klasse.
 *
 * Deze destructor sluit de socketverbinding wanneer het SocketClient-object wordt
 * vernietigd. Hierdoor blijft er geen open socket achter.
 */
SocketClient::~SocketClient() {
    closeSocket();
}

/*!
 * @brief Maakt een TCP-verbinding met de ingestelde server.
 *
 * Deze functie maakt eerst een TCP-socket aan. Daarna wordt het IP-adres omgezet
 * naar een netwerkadres met inet_pton(). Vervolgens probeert de functie verbinding
 * te maken met de server via connect().
 *
 * Als de verbinding succesvol is, wordt een ontvangsttimeout ingesteld. Hierdoor
 * blijft receiveMessage() niet onnodig lang wachten wanneer er geen bericht
 * beschikbaar is.
 *
 * @return true als de verbinding succesvol is gemaakt, anders false.
 */
bool SocketClient::connectToServer()
{
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return false;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(portNumber);

    if (inet_pton(AF_INET, ipAddress.c_str(), &addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sock);
        sock = -1;
        return false;
    }

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        close(sock);
        sock = -1;
        return false;
    }

    setReceiveTimeout(200);

    return true;
}

/*!
 * @brief Stelt een timeout in voor het ontvangen van berichten.
 *
 * Deze functie stelt met setsockopt() een ontvangsttimeout in voor de socket.
 * Daardoor wacht de socket maximaal het opgegeven aantal milliseconden op
 * binnenkomende data.
 *
 * @param milliseconds Timeoutwaarde in milliseconden.
 * @return true als de timeout succesvol is ingesteld, anders false.
 */
bool SocketClient::setReceiveTimeout(long milliseconds)
{
    if (sock < 0) return false;
    struct timeval timeout;
    timeout.tv_sec  = milliseconds / 1000;
    timeout.tv_usec = (milliseconds % 1000) * 1000;
    return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == 0;
}

/*!
 * @brief Verstuurt een tekstbericht via de TCP-verbinding.
 *
 * Deze functie controleert eerst of er een geldige socket actief is. Daarna wordt
 * met een mutex voorkomen dat meerdere threads tegelijk via dezelfde socket
 * schrijven.
 *
 * Aan het bericht wordt automatisch een newline toegevoegd, zodat de ontvangende
 * kant het bericht als volledige regel kan verwerken.
 *
 * @param msg Het bericht dat verstuurd moet worden.
 * @return Het aantal verzonden bytes, of -1 als er geen actieve socket is of bij een fout.
 */
ssize_t SocketClient::sendMessage(const std::string& msg)
{
    if (sock < 0) return -1;
    std::lock_guard<std::mutex> lock(sendMutex);
    std::string withNewline = msg + "\n";
    ssize_t bytes = send(sock, withNewline.c_str(), withNewline.length(), 0);
    std::cout << "[" << name << "] Verzonden: '" << msg << "' bytes=" << bytes << std::endl;
    return bytes;
}

/*!
 * @brief Ontvangt één bericht via de TCP-verbinding.
 *
 * Deze functie leest data van de actieve socket en zet deze data om naar een
 * string. Eventuele newline- of carriage return-tekens aan het einde van het
 * bericht worden verwijderd.
 *
 * Binnen Raspberry Pi-A wordt deze functie gebruikt om berichten te ontvangen
 * van bijvoorbeeld Raspberry Pi-B of een Wemos-module.
 *
 * @param msg Referentie naar de string waarin het ontvangen bericht wordt opgeslagen.
 * @return Het aantal ontvangen bytes, 0 als de verbinding gesloten is, of -1 bij een fout.
 */
ssize_t SocketClient::receiveMessage(std::string& msg)
{
    msg = "";
    if (sock < 0) return -1;

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    ssize_t len = read(sock, buffer, sizeof(buffer));

    if (len > 0)
    {
        msg = std::string(buffer, len);
        
        while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r'))
        {
            msg.pop_back();
        }
    }

    return len;
}

/*!
 * @brief Sluit de actieve socketverbinding.
 *
 * Deze functie controleert of er een geldige socket actief is. Als dat zo is,
 * wordt de socket gesloten en wordt de socketwaarde teruggezet naar -1.
 */
void SocketClient::closeSocket()
{
    if (sock >= 0)
    {
        close(sock);
        sock = -1;
    }
}

/*!
 * @brief Luistert continu naar binnenkomende berichten.
 *
 * Deze functie blijft in een lus berichten ontvangen via de socket. Ontvangen
 * data wordt gesplitst op newline-tekens, zodat meerdere berichten correct als
 * losse regels verwerkt kunnen worden.
 *
 * Voor elk volledig ontvangen bericht wordt de callbackfunctie onMessage()
 * aangeroepen.
 *
 * Deze functie wordt gebruikt wanneer Raspberry Pi-A continu moet luisteren naar
 * berichten van bijvoorbeeld Wemos-modules of Raspberry Pi-B.
 *
 * @param onMessage Callbackfunctie die wordt uitgevoerd bij elk ontvangen bericht.
 */
void SocketClient::receiveLoop(std::function<void(const std::string&)> onMessage)
{
    char buffer[1024];
    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes = recv(sock, buffer, sizeof(buffer), 0);

        if (bytes > 0)
        {

            std::string chunk(buffer, bytes);
            size_t start = 0;
            for (size_t i = 0; i <= chunk.size(); ++i)
            {
                if (i == chunk.size() || chunk[i] == '\n')
                {
                    std::string line = chunk.substr(start, i - start);
                    while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
                        line.pop_back();

                    if (!line.empty())
                    {
                        std::cout << "[" << name << "] Ontvangen: '" << line << "'" << std::endl;
                        onMessage(line);
                    }
                    start = i + 1;
                }
            }
        }
        else if (bytes == 0)
        {
            std::cerr << "[" << name << "] verbinding gesloten." << std::endl;
            break;
        }

    }
}