/*!
 * @file wifiservermanager.cpp
 * @brief Implementatie van de WifiServerManager-klasse.
 *
 * Dit bestand bevat de functies voor het opzetten en beheren van de WiFi-
 * en TCP-servercommunicatie op de Wemos D1 Mini.
 *
 * Binnen de RFID-module wordt deze klasse gebruikt om een bericht naar
 * Raspberry Pi-A te sturen wanneer een geldige RFID-chip wordt herkend.
 *
 * @author Quinten van Ewijk
 * @date 2026
 */

#include "WifiServerManager.h"

/*!
 * @brief Constructor van de WifiServerManager-klasse.
 *
 * Deze constructor slaat de SSID en het wachtwoord van het WiFi-netwerk op
 * en initialiseert de TCP-server op poort 6767.
 *
 * @param s De naam van het WiFi-netwerk waarmee de Wemos verbinding maakt.
 * @param p Het wachtwoord van het WiFi-netwerk.
 */
WifiServerManager::WifiServerManager(const char* s, const char* p) : server(6767)
{
    ssid = s;
    password = p;
}

/*!
 * @brief Start de WiFi-verbinding en de TCP-server.
 *
 * Deze functie maakt verbinding met het ingestelde WiFi-netwerk. Zolang de
 * Wemos nog niet verbonden is, blijft de functie wachten.
 *
 * Wanneer de verbinding actief is, wordt de TCP-server gestart en worden
 * een verbindingsmelding en het IP-adres naar de seriële monitor geschreven.
 */
void WifiServerManager::begin() {
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    server.begin();

    Serial.println("WiFi connected");
    Serial.println(WiFi.localIP());
}

/*!
 * @brief Controleert of er een client verbonden is.
 *
 * Deze functie kijkt of er nog geen actieve client is of dat de vorige client
 * niet meer verbonden is. In dat geval wordt gecontroleerd of een nieuwe client
 * verbinding wil maken met de TCP-server.
 */
void WifiServerManager::checkClient() {
    if (!client || !client.connected()) {
        client = server.available();
    }
}

/*!
 * @brief Verstuurt een tekstbericht naar de verbonden client.
 *
 * Deze functie controleert eerst of er een client verbonden is. Als dat zo is,
 * wordt het meegegeven bericht via de TCP-verbinding verstuurd.
 *
 * Het bericht wordt ook naar de seriële monitor geschreven voor debugging.
 *
 * @param message Het bericht dat naar Raspberry Pi-A verstuurd moet worden.
 */
void WifiServerManager::sendMessage(String message) {
    if (client && client.connected()) {
        client.println(message);
    }

    Serial.println(message);
}