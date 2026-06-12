/*!
 * @file wifiservermanager.h
 * @brief Declaratie van de WifiServerManager-klasse.
 *
 * Dit headerbestand bevat de klasse die verantwoordelijk is voor het beheren
 * van de WiFi-verbinding en TCP-servercommunicatie op de Wemos D1 Mini.
 *
 * Binnen de RFID-module wordt deze klasse gebruikt om na herkenning van een
 * RFID-chip een bericht naar Raspberry Pi-A te sturen.
 *
 * @author Quinten van Ewijk
 * @date 2026
 */

#ifndef WIFISERVERMANAGER_H
#define WIFISERVERMANAGER_H

#include <ESP8266WiFi.h>
#include <Arduino.h>

/*!
 * @class WifiServerManager
 * @brief Klasse voor WiFi- en TCP-servercommunicatie op de Wemos D1 Mini.
 *
 * Deze klasse maakt verbinding met een opgegeven WiFi-netwerk en start een
 * TCP-server op poort 6767. Raspberry Pi-A kan als client verbinding maken
 * met deze server. De Wemos kan daarna berichten zoals "RFID:1" naar de
 * verbonden client sturen.
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
     * De server luistert op poort 6767 en wacht op een clientverbinding
     * vanuit Raspberry Pi-A.
     */
    WiFiServer server;

    /*!
     * @brief Clientobject voor de actieve TCP-verbinding.
     *
     * Dit object stelt de verbonden client voor. Via dit object worden
     * berichten verstuurd naar Raspberry Pi-A.
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
     * Deze functie maakt verbinding met het ingestelde WiFi-netwerk en start
     * daarna de TCP-server.
     */
    void begin();

    /*!
     * @brief Controleert of er een client verbonden is.
     *
     * Deze functie kijkt of er al een actieve clientverbinding bestaat. Als
     * er geen client verbonden is, wordt gecontroleerd of een nieuwe client
     * beschikbaar is.
     */
    void checkClient();

    /*!
     * @brief Verstuurt een bericht naar de verbonden client.
     *
     * @param message Het bericht dat naar Raspberry Pi-A verstuurd moet worden.
     */
    void sendMessage(String message);
};

#endif