/*!
 * @file tcpsocketrasberry.cpp
 * @brief Implementatie van de TcpSocketRasberry-klasse voor Raspberry Pi-B.
 *
 * Dit bestand bevat de TCP-serverlogica van Raspberry Pi-B binnen het PES
 * restaurantproject. Raspberry Pi-B ontvangt via TCP berichten van Raspberry Pi-A
 * en vertaalt deze berichten naar CANBUS-berichten richting STM32-microcontrollers.
 *
 * Voorbeelden van ontvangen TCP-berichten zijn tafeloproepen, CO2-statussen en
 * RFID-statussen. Deze worden in handleMessage() omgezet naar de juiste CAN-ID's
 * en databytes.
 *
 * Daarnaast kan Raspberry Pi-B via sendToClient() berichten terugsturen naar
 * Raspberry Pi-A, bijvoorbeeld wanneer een STM32-module via CANBUS meldt dat een
 * tafeloproep is gereset!
 *
 * @author Arbër Deda
 * @author Quinten van Ewijk
 * @date 2026
 */

#include "tcpsocketrasberry.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h> 
#include <string>

/*!
 * @brief Constructor van de TcpSocketRasberry-klasse.
 *
 * Deze constructor initialiseert de TCP-server op het opgegeven poortnummer.
 * Er wordt een TCP-socket aangemaakt, waarna deze socket op non-blocking mode
 * wordt gezet. Hierdoor blijft het programma niet onnodig vastlopen tijdens het
 * wachten op een clientverbinding.
 *
 * Ook worden socketopties ingesteld zodat het adres en de poort opnieuw gebruikt
 * kunnen worden. Daarna wordt de serveradresstructuur gevuld met het IPv4-type,
 * alle beschikbare netwerkinterfaces en het ingestelde poortnummer.
 *
 * @param port Poortnummer waarop Raspberry Pi-B luistert voor verbindingen van Raspberry Pi-A.
 */
TcpSocketRasberry::TcpSocketRasberry(int port)
    : _port(port), _opt(1), _client_socket(-1)
{
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int flags = fcntl(_server_fd, F_GETFL, 0);
    fcntl(_server_fd, F_SETFL, flags | O_NONBLOCK);

    setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &_opt, sizeof(_opt));

    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(_port);
}

/*!
 * @brief Destructor van de TcpSocketRasberry-klasse.
 *
 * Deze destructor sluit de geopende serversocket en de actieve clientsocket
 * wanneer deze nog open staan. Hierdoor worden de netwerkresources netjes
 * vrijgegeven wanneer het object wordt vernietigd.
 */
TcpSocketRasberry::~TcpSocketRasberry()
{
    if (_server_fd >= 0) {
        close(_server_fd);
    }

    if (_client_socket >= 0) {
        close(_client_socket);
    }
}

/*!
 * @brief Start de TCP-server op Raspberry Pi-B.
 *
 * Deze functie bindt de serversocket aan het ingestelde IP-adres en poortnummer.
 * Daarna wordt de socket in luistermodus gezet, zodat Raspberry Pi-A verbinding
 * kan maken met Raspberry Pi-B.
 *
 * Als bind() of listen() mislukt, wordt een foutmelding weergegeven en geeft de
 * functie false terug.
 *
 * @return true als de TCP-server succesvol gestart is, anders false.
 */
bool TcpSocketRasberry::start()
{
    if (bind(_server_fd, (struct sockaddr*)&_address, sizeof(_address)) < 0) {
        perror("Bind gefaald");
        return false;
    }

    if (listen(_server_fd, 3) < 0) {
        perror("Listen gefaald");
        return false;
    }

    return true;
}

/*!
 * @brief Voert de hoofdloop van de TCP-server uit.
 *
 * Deze functie blijft continu draaien en wacht op een verbinding van Raspberry Pi-A.
 * Wanneer er nog geen client verbonden is, probeert de server een nieuwe client
 * te accepteren met accept().
 *
 * Zodra Raspberry Pi-A verbonden is, leest de functie inkomende berichten vanaf
 * de clientsocket. Ontvangen berichten worden doorgestuurd naar handleMessage(),
 * waar ze worden verwerkt en eventueel worden omgezet naar CANBUS-berichten.
 *
 * Als de clientverbinding wordt verbroken, wordt de clientsocket gesloten en kan
 * Raspberry Pi-B opnieuw wachten op een nieuwe verbinding.
 */
void TcpSocketRasberry::run()
{
    int addrlen = sizeof(_address);
    char buffer[1024];

    while (true)
    {
        if (_client_socket < 0)
        {
            _client_socket = accept(_server_fd,
                                    (struct sockaddr*)&_address,
                                    (socklen_t*)&addrlen);

            if (_client_socket < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    usleep(10000);
                    continue;
                }

                std::cerr << "Accept fout: "
                        << strerror(errno)
                        << std::endl;

                continue;
            }

            std::cout << "Client verbonden!" << std::endl;
        }

        memset(buffer, 0, sizeof(buffer));

        ssize_t bytesRead = read(_client_socket, buffer, 1024);

        if (bytesRead > 0)
        {
            handleMessage(_client_socket, buffer);
        }
        else if (bytesRead == 0)
        {
            std::cout << "Client disconnected" << std::endl;

            close(_client_socket);
            _client_socket = -1;
        }
    }
}

/*!
 * @brief Verwerkt een ontvangen TCP-bericht van Raspberry Pi-A.
 *
 * Deze functie ontvangt een tekstbericht vanaf Raspberry Pi-A en verwijdert eerst
 * eventuele newline- en carriage return-tekens aan het einde van het bericht.
 * Daarna wordt het opgeschoonde bericht vergeleken met bekende commando's.
 *
 * Afhankelijk van het ontvangen commando wordt een CANBUS-bericht verstuurd naar
 * de juiste STM32-module.
 *
 * De belangrijkste omzettingen zijn:
 * - "tafel1:1" naar CAN-ID 0x300 met data 0x01;
 * - "tafel2:1" naar CAN-ID 0x301 met data 0x01;
 * - "co2:1" naar CAN-ID 0x101 met data 0x01;
 * - "co2:0" naar CAN-ID 0x101 met data 0x00;
 * - "RFID:1" naar CAN-ID 0x501 met data 0x01;
 * - "RFID:0" naar CAN-ID 0x501 met data 0x00.
 *
 * Berichten zoals "tafel1:0" en "tafel2:0" worden niet opnieuw naar CANBUS
 * gestuurd. Dit voorkomt een feedback-loop tussen Raspberry Pi-A, Raspberry Pi-B
 * en de STM32-modules.
 *
 * @param client_socket De actieve TCP-clientsocket van Raspberry Pi-A.
 * @param message Het ontvangen tekstbericht dat verwerkt moet worden.
 */
void TcpSocketRasberry::handleMessage(int client_socket, const char* message)
{
    std::string msg(message);

    while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r'))
    {
        msg.pop_back();
    }

    std::cout << "Ontvangen raw: '" << message << "'" << std::endl;
    std::cout << "Ontvangen clean: '" << msg << "'" << std::endl;

    std::string reply = "Bericht ontvangen!";
    const char* interface = "can0";

    if (msg == "tafel1:1")
    {
        std::cout << "Actie: LED inschakelen op Raspberry Pi" << std::endl;

        reply = "tafel1:1";

        if (_canBus.open(interface))
        {
            std::cout << "Succes: Verbonden met fysieke bus (" << interface << ")" << std::endl;

            uint8_t data[] = {0x01};

            if (_canBus.send(0x300, data, 1) > 0)
            {
                std::cout << "Bericht verzonden naar de bus." << std::endl;
            }
            else
            {
                std::cerr << "Verzenden mislukt: " << std::strerror(errno) << std::endl;
            }
        }
    }

else if (msg == "tafel2:1")
{
std::cout << "Actie: Tafel 2 LED inschakelen op Raspberry Pi" << std::endl;

reply = "tafel2:1";

if (_canBus.open(interface))
{
    std::cout << "Succes: Verbonden met fysieke bus (" << interface << ")" << std::endl;

    uint8_t data[] = {0x01};

    if (_canBus.send(0x301, data, 1) > 0)
    {
        std::cout << "Bericht verzonden naar de bus voor tafel 2." << std::endl;
    }
    else
    {
        std::cerr << "Verzenden mislukt: " << std::strerror(errno) << std::endl;
    }
 }   
}
    else if (msg == "tafel1:0")
    {
        std::cout << "tafel1:0 ontvangen op PI-B, niet naar CAN gestuurd om feedback-loop te voorkomen." << std::endl;

        reply = "tafel1:0 genegeerd";
    }
    else if (msg == "tafel2:0")
    {
        std::cout << "tafel2:0 ontvangen op PI-B, niet naar CAN gestuurd om feedback-loop te voorkomen." << std::endl;

        reply = "tafel2:0 genegeerd";
    }
else if (msg == "co2:1")
{
    std::cout << "Actie: CO2 te hoog, ventilatie AAN via CAN." << std::endl;

    reply = "co2 ontvangen";

    if (_canBus.open(interface))
    {
        uint8_t data[] = {0x01};

        if (_canBus.send(0x101, data, 1) > 0)
        {
            std::cout << "CAN 0x101 data 01 verzonden. Ventilatie/LED AAN." << std::endl;
        }
        else
        {
            std::cerr << "CO2 verzenden mislukt: " << std::strerror(errno) << std::endl;
        }
    }
}
else if (msg == "co2:0")
{
    std::cout << "Actie: CO2 normaal, ventilatie UIT via CAN." << std::endl;

    reply = "co2 ontvangen";

    if (_canBus.open(interface))
    {
        uint8_t data[] = {0x00};

        if (_canBus.send(0x101, data, 1) > 0)
        {
            std::cout << "CAN 0x101 data 00 verzonden. Ventilatie/LED UIT." << std::endl;
        }
        else
        {
            std::cerr << "CO2 verzenden mislukt: " << std::strerror(errno) << std::endl;
        }
    }
}

else if (msg == "RFID:1")
{
std::cout << "Actie: RFID toegestaan, stuur CAN 0x501 data 01." << std::endl;

reply = "RFID ontvangen";

if (_canBus.open(interface))
{
    uint8_t data[] = {0x01};

    if (_canBus.send(0x501, data, 1) > 0)
    {
        std::cout << "CAN 0x501 data 01 verzonden. Bovenste servo naar links." << std::endl;
    }
    else
    {
        std::cerr << "RFID verzenden mislukt: " << std::strerror(errno) << std::endl;
    }
}
}
else if (msg == "RFID:0")
{
std::cout << "Actie: RFID uit/reset, stuur CAN 0x501 data 00." << std::endl;

reply = "RFID ontvangen";

if (_canBus.open(interface))
{
    uint8_t data[] = {0x00};

    if (_canBus.send(0x501, data, 1) > 0)
    {
        std::cout << "CAN 0x501 data 00 verzonden. Bovenste servo reset/uit." << std::endl;
    }
    else
    {
        std::cerr << "RFID verzenden mislukt: " << std::strerror(errno) << std::endl;
    }
}
}

    else
    {
        std::cout << "Onbekend bericht ontvangen: '" << msg << "'" << std::endl;
    }

    std::string replyWithNewline = reply + "\n";
    send(client_socket, replyWithNewline.c_str(), replyWithNewline.length(), 0);
}

/*!
 * @brief Stuurt een bericht terug naar Raspberry Pi-A.
 *
 * Deze functie verstuurt een tekstbericht via de actieve TCP-verbinding naar
 * Raspberry Pi-A. De mutex zorgt ervoor dat niet meerdere delen van het programma
 * tegelijk naar dezelfde socket schrijven.
 *
 * Aan het bericht wordt automatisch een newline toegevoegd, zodat Raspberry Pi-A
 * het bericht als volledige regel kan uitlezen.
 *
 * Deze functie wordt onder andere gebruikt wanneer Raspberry Pi-B een bericht van
 * een STM32-module ontvangt en dit terug moet sturen naar Raspberry Pi-A.
 *
 * @param message Het bericht dat naar Raspberry Pi-A verstuurd moet worden.
 */
void TcpSocketRasberry::sendToClient(const std::string& message)
{
    std::lock_guard<std::mutex> lock(_socketMutex);

    if (_client_socket >= 0)
    {
        std::string messageWithNewline = message + "\n";

        send(_client_socket, messageWithNewline.c_str(), messageWithNewline.length(), 0);

        std::cout << "Naar PI-A gestuurd: '" << messageWithNewline << "'" << std::endl;
    }
}