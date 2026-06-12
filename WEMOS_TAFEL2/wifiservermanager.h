/*!
 * @file wifiservermanager.h
 * @brief Declaratie van de WifiServerManager-klasse.
 *
 * Dit headerbestand bevat de klasse die verantwoordelijk is voor het beheren
 * van de WiFi-verbinding en TCP-servercommunicatie op de Wemos D1 Mini.
 *
 * Binnen de tafel 2-module wordt deze klasse gebruikt om klantoproepen naar
 * Raspberry Pi-A te sturen en resetberichten van Raspberry Pi-A te ontvangen.
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
 * TCP-server op poort 9091. Raspberry Pi-A kan als client verbinden met deze
 * Wemos.
 *
 * De Wemos gebruikt deze klasse om berichten zoals "tafel2:1" te versturen
 * wanneer een klant hulp vraagt. Daarnaast kan de Wemos berichten zoals
 * "tafel2:0" ontvangen om de LED bij de tafel uit te schakelen.
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
     * De server luistert op poort 9091 en wacht op een verbinding van
     * Raspberry Pi-A.
     */
    WiFiServer server;

    /*!
     * @brief Clientobject voor de actieve TCP-verbinding.
     *
     * Via dit object worden berichten naar Raspberry Pi-A gestuurd en
     * berichten van Raspberry Pi-A gelezen.
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
     */
    void begin();

    /*!
     * @brief Controleert of er een client verbonden is.
     */
    void checkClient();

    /*!
     * @brief Verstuurt een bericht naar de verbonden client.
     *
     * @param message Het bericht dat verstuurd moet worden.
     */
    void sendMessage(String message);

    /*!
     * @brief Leest een binnenkomend bericht van de verbonden client.
     *
     * Deze functie wordt gebruikt om berichten zoals "tafel2:0" van
     * Raspberry Pi-A te ontvangen.
     *
     * @return Het ontvangen bericht als String, of een lege String als er geen
     * bericht beschikbaar is.
     */
    String readMessage();
};

#endif