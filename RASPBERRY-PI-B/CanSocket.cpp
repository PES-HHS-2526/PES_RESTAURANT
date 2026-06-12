/*!
 * @file CanSocket.cpp
 * @brief Implementatie van de CanSocket-klasse voor CANBUS-communicatie op Raspberry Pi-B.
 *
 * Dit bestand bevat de implementatie van de functies waarmee Raspberry Pi-B
 * via een Linux CAN-interface, zoals "can0", CANBUS-berichten kan verzenden
 * en ontvangen.
 *
 * Binnen het PES restaurantproject gebruikt Raspberry Pi-B deze klasse om
 * berichten tussen Raspberry Pi-A en STM32-microcontrollers door te sturen.
 * TCP-berichten die vanaf Raspberry Pi-A binnenkomen, worden in main.cpp
 * vertaald naar CAN-berichten. Daarnaast kunnen ontvangen CAN-berichten van
 * STM32-modules worden teruggestuurd naar Raspberry Pi-A.
 *
 * @author Arbër Deda
 * @author Quinten van Ewijk
 * @date 2026
 */

#include "CanSocket.h"
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

/*!
 * @brief Constructor van de CanSocket-klasse.
 *
 * Deze constructor initialiseert het socketbestand naar -1, zodat duidelijk is
 * dat er bij het aanmaken van het object nog geen geldige CAN-socket geopend is.
 *
 * Daarnaast worden de adresstructuur en interface request-structuur leeg gemaakt
 * met std::memset. Hierdoor bevatten deze structuren geen oude of willekeurige
 * data voordat ze gebruikt worden bij het openen van de CAN-interface.
 */
CanSocket::CanSocket() : _socket_fd(-1) {
    std::memset(&_addr, 0, sizeof(_addr));
    std::memset(&_ifr, 0, sizeof(_ifr));
}

/*!
 * @brief Destructor van de CanSocket-klasse.
 *
 * Deze destructor roept close() aan om ervoor te zorgen dat een eventueel
 * geopende CAN-socket netjes wordt gesloten wanneer het object wordt vernietigd.
 */
CanSocket::~CanSocket() {
    close();
}

/*!
 * @brief Opent en configureert een Linux CAN-socket.
 *
 * Deze functie maakt een RAW CAN-socket aan en koppelt deze aan de opgegeven
 * CAN-interface, bijvoorbeeld "can0".
 *
 * De socket wordt op non-blocking mode gezet, zodat receive() het programma
 * niet onnodig blijft blokkeren wanneer er geen CAN-frame beschikbaar is.
 * Daarnaast wordt een receive-timeout ingesteld.
 *
 * Vervolgens wordt de interface-index van de opgegeven CAN-interface opgezocht
 * met ioctl(). Deze index wordt gebruikt om de socket aan de juiste interface
 * te binden.
 *
 * Als een stap mislukt, wordt de socket gesloten en geeft de functie false terug.
 *
 * @param interface Naam van de CAN-interface die geopend moet worden, bijvoorbeeld "can0".
 * @return true als de CAN-socket succesvol is geopend en gebonden, anders false.
 */
bool CanSocket::open(const std::string& interface) {
    _socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (_socket_fd < 0) return false;

    int flags = fcntl(_socket_fd, F_GETFL, 0);
    fcntl(_socket_fd, F_SETFL, flags | O_NONBLOCK);

    struct timeval tv;
    tv.tv_sec = 1; 
    tv.tv_usec = 0;
    setsockopt(_socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    std::strncpy(_ifr.ifr_name, interface.c_str(), IFNAMSIZ);
    if (ioctl(_socket_fd, SIOCGIFINDEX, &_ifr) < 0) {
        close();
        return false;
    }

    _addr.can_family = AF_CAN;
    _addr.can_ifindex = _ifr.ifr_ifindex;

    if (bind(_socket_fd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0) {
        close();
        return false;
    }
    return true;
}

/*!
 * @brief Sluit de geopende CAN-socket.
 *
 * Deze functie controleert eerst of er een geldige socket open staat.
 * Als dat zo is, wordt de socket gesloten met de globale close()-functie
 * uit unistd.h. Daarna wordt de file descriptor teruggezet naar -1.
 *
 * Hierdoor kan later gecontroleerd worden dat er geen actieve CAN-socket meer
 * geopend is.
 */
void CanSocket::close() {
    if (_socket_fd != -1) {
        ::close(_socket_fd);
        _socket_fd = -1;
    }
}

/*!
 * @brief Verstuurt een CAN-frame via de geopende CAN-interface.
 *
 * Deze functie maakt een can_frame aan, vult het CAN-ID en de databytes in en
 * schrijft het frame daarna naar de CAN-socket.
 *
 * CAN Classic ondersteunt maximaal 8 databytes per frame. Daarom wordt de
 * lengte begrensd op maximaal 8 bytes.
 *
 * Binnen het PES restaurantproject wordt deze functie gebruikt om vanuit
 * Raspberry Pi-B commando's naar STM32-modules te sturen, bijvoorbeeld voor
 * tafeloproepen, CO2-ventilatie of RFID-deuraansturing.
 *
 * @param id Het CAN-ID waarmee het bericht verzonden moet worden.
 * @param data Pointer naar de data die in het CAN-frame gezet moet worden.
 * @param len Aantal databytes dat verzonden moet worden.
 * @return Het aantal geschreven bytes bij succes, of -1 bij een fout.
 */
int CanSocket::send(uint32_t id, const uint8_t* data, uint8_t len) {
    struct can_frame frame;
    std::memset(&frame, 0, sizeof(frame));
    
    frame.can_id = id;
    frame.can_dlc = (len > 8) ? 8 : len;
    std::memcpy(frame.data, data, frame.can_dlc);

    return write(_socket_fd, &frame, sizeof(struct can_frame));
}

/*!
 * @brief Ontvangt een CAN-frame via de geopende CAN-interface.
 *
 * Deze functie leest een CAN-frame van de socket en plaatst het resultaat in
 * het meegegeven frame-object.
 *
 * Binnen Raspberry Pi-B wordt deze functie gebruikt om berichten van
 * STM32-modules te ontvangen. Een voorbeeld hiervan is een resetbericht vanuit
 * de keukenmodule waarmee de oproep van een tafel wordt afgehandeld.
 *
 * @param frame Referentie naar het can_frame-object waarin het ontvangen frame wordt opgeslagen.
 * @return Het aantal gelezen bytes bij succes, of -1 bij een fout.
 */
int CanSocket::receive(struct can_frame& frame) {
    return read(_socket_fd, &frame, sizeof(struct can_frame));
}