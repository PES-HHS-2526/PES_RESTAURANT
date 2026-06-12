/*!
 * @file WifiServerManager.h
 * @brief Declaratie van de WifiServerManager-klasse.
 *
 * Dit headerbestand bevat de klasse die verantwoordelijk is voor het beheren
 * van de WiFi-verbinding en TCP-servercommunicatie op de Wemos D1 Mini.
 *
 * Binnen de Wemos-lichtkrant wordt deze klasse gebruikt om berichten van
 * Raspberry Pi-A te ontvangen. Deze berichten worden daarna vertaald naar
 * acties op de LED-lichtkrant.
 *
 * @author Haki Abdulovski
 * @date 2026
 */

#ifndef WIFISERVERMANAGER_H
#define WIFISERVERMANAGER_H

#include <ESP8266WiFi.h>

/*!
 * @class WifiServerManager
 * @brief Klasse voor WiFi- en TCP-servercommunicatie op de Wemos D1 Mini.
 *
 * Deze klasse maakt verbinding met een opgegeven WiFi-netwerk en start een
 * TCP-server op een opgegeven poort. Raspberry Pi-A kan als client verbinding
 * maken met deze server en tekstberichten of commando's naar de Wemos sturen.
 *
 * Binnen de lichtkrantmodule worden ontvangen berichten gebruikt om tekst te
 * tonen, de scrollsnelheid aan te passen of de display leeg te maken.
 */
class WifiServerManager {
private:
    /*!
     * @brief Naam van het WiFi-netwerk.
     *
     * Deze SSID wordt gebruikt om de Wemos D1 Mini met het juiste netwerk
     * te verbinden.
     */
    const char* ssid;

    /*!
     * @brief Wachtwoord van het WiFi-netwerk.
     *
     * Dit wachtwoord hoort bij de opgegeven SSID.
     */
    const char* password;

    /*!
     * @brief TCP-serverobject voor inkomende verbindingen.
     *
     * De server luistert op de ingestelde poort en wacht op verbindingen
     * vanuit Raspberry Pi-A.
     */
    WiFiServer server;

    /*!
     * @brief Clientobject voor de actieve TCP-verbinding.
     *
     * Dit object stelt de verbonden client voor. Via dit object worden
     * berichten gelezen en eventueel verstuurd.
     */
    WiFiClient client;

public:
    /*!
     * @brief Constructor voor het instellen van netwerkgegevens en serverpoort.
     *
     * @param s De SSID van het WiFi-netwerk.
     * @param p Het wachtwoord van het WiFi-netwerk.
     * @param port De TCP-poort waarop de Wemos-server luistert.
     */
    WifiServerManager(const char* s, const char* p, uint16_t port = 5555);

    /*!
     * @brief Start de WiFi-verbinding en de TCP-server.
     *
     * Deze functie maakt verbinding met het ingestelde WiFi-netwerk en start
     * daarna de TCP-server.
     */
    void begin();

    /*!
     * @brief Controleert of er een client verbonden is.
     *
     * Deze functie controleert of de huidige client nog actief is of dat er
     * een nieuwe clientverbinding beschikbaar is.
     */
    void checkClient();

    /*!
     * @brief Verstuurt een bericht naar de verbonden client.
     *
     * @param message Het bericht dat naar de client verstuurd moet worden.
     */
    void sendMessage(String message);

    /*!
     * @brief Leest een binnenkomend bericht van de verbonden client.
     *
     * Deze functie leest een bericht van de actieve client, bijvoorbeeld
     * "SPEED:100" of een tekstbericht voor de lichtkrant.
     *
     * @return Het ontvangen bericht als String, of een lege String als er
     * geen bericht beschikbaar is.
     */
    String readMessage();
};

#endif