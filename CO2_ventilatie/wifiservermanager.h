/*!
 * @file WifiServerManager.h
 * @brief Declaratie van de WifiServerManager-klasse.
 *
 * Dit headerbestand bevat de klasse die verantwoordelijk is voor het beheren
 * van de WiFi-verbinding en TCP-servercommunicatie op de Wemos D1 Mini.
 *
 * De klasse wordt gebruikt om berichten tussen de Wemos en Raspberry Pi-A
 * uit te wisselen binnen het PES restaurantproject.
 *
 * @author Arbër Deda
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
 * TCP-server. Raspberry Pi-A kan als client verbinding maken met deze server
 * om berichten te ontvangen of te versturen.
 *
 * Binnen de CO2-ventilatiemodule wordt deze klasse gebruikt om berichten zoals
 * "co2:1" en "co2:0" naar Raspberry Pi-A te sturen.
 */
class WifiServerManager {
private:
    /*!
     * @brief Naam van het WiFi-netwerk.
     *
     * Deze SSID wordt gebruikt om de Wemos D1 Mini met het juiste netwerk te verbinden.
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
     * De server luistert op poort 9092 en wacht op een clientverbinding
     * vanuit Raspberry Pi-A.
     */
    WiFiServer server;

    /*!
     * @brief Clientobject voor de actieve TCP-verbinding.
     *
     * Dit object stelt de verbonden client voor. Via dit object worden berichten
     * gelezen en verstuurd.
     */
    WiFiClient client;

public:
    /*!
     * @brief Constructor voor het instellen van netwerkgegevens.
     *
     * @param s De SSID van het WiFi-netwerk.
     * @param p Het wachtwoord van het WiFi-netwerk.
     */
    WifiServerManager(const char* s, const char* p);

    /*!
     * @brief Start de WiFi-verbinding en de TCP-server.
     *
     * @return Geen returnwaarde.
     */
    void begin();

    /*!
     * @brief Controleert of er een client verbonden is.
     *
     * @return Geen returnwaarde.
     */
    void checkClient();

    /*!
     * @brief Verstuurt een bericht naar de verbonden client.
     *
     * @param message Het bericht dat verstuurd moet worden.
     * @return Geen returnwaarde.
     */
    void sendMessage(String message);

    /*!
     * @brief Leest een bericht van de verbonden client.
     *
     * @return Het ontvangen bericht als String, of een lege String als er geen
     * bericht beschikbaar is.
     */
    String readMessage();
};

#endif