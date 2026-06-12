/*!
 * @file WifiServerManager.cpp
 * @brief Implementatie van de WifiServerManager-klasse.
 *
 * Dit bestand bevat de functies voor het opzetten en beheren van de WiFi-
 * en TCP-servercommunicatie op de Wemos D1 Mini.
 *
 * Binnen de Wemos-lichtkrantmodule zorgt deze klasse ervoor dat Raspberry Pi-A
 * verbinding kan maken en berichten kan sturen naar de Wemos. Deze berichten
 * worden vervolgens gebruikt om de tekst of instellingen van de lichtkrant aan
 * te passen.
 *
 * @author Haki Abdulovski
 * @date 2026
 */

#include "WifiServerManager.h"

/*!
 * @brief Constructor van de WifiServerManager-klasse.
 *
 * Deze constructor slaat de SSID en het wachtwoord van het WiFi-netwerk op
 * en initialiseert de TCP-server op de meegegeven poort.
 *
 * @param s De naam van het WiFi-netwerk waarmee de Wemos verbinding maakt.
 * @param p Het wachtwoord van het WiFi-netwerk.
 * @param port De TCP-poort waarop de server luistert.
 */
WifiServerManager::WifiServerManager(const char* s, const char* p, uint16_t port) : server(port)
{
    ssid = s;
    password = p;
}

/*!
 * @brief Start de WiFi-verbinding en de TCP-server.
 *
 * Deze functie stelt eerst de hostname van de Wemos in op "WemosLichtkrant".
 * Daarna wordt verbinding gemaakt met het opgegeven WiFi-netwerk.
 *
 * Zolang de Wemos nog niet verbonden is, wordt via de seriële monitor een punt
 * weergegeven. Zodra de verbinding actief is, wordt de TCP-server gestart en
 * wordt het IP-adres van de Wemos getoond.
 */
void WifiServerManager::begin() {
    WiFi.hostname("WemosLichtkrant");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    server.begin();

    Serial.println();
    Serial.println("WiFi connected");
    Serial.println(WiFi.localIP());
}

/*!
 * @brief Controleert en beheert de actieve clientverbinding.
 *
 * Als er nog data klaarstaat van de huidige client, wordt de client niet
 * vervangen. Hierdoor wordt voorkomen dat een bericht verloren gaat.
 *
 * Als er geen actieve client is, controleert de functie of er een nieuwe
 * client verbinding wil maken met de TCP-server. Wanneer een client verbonden
 * is, wordt dit gemeld via de seriële monitor.
 */
void WifiServerManager::checkClient()
{
    // Als er nog data klaarstaat van de huidige client, niet vervangen
    if(client && client.available())
    {
        return;
    }

    if(!client || !client.connected())
    {
        WiFiClient newClient = server.available();

        if(newClient)
        {
            client = newClient;
            Serial.println("Client connected!");
        }
    }
}

/*!
 * @brief Verstuurt een tekstbericht naar de verbonden client.
 *
 * Deze functie controleert eerst of er een client verbonden is. Als dat zo is,
 * wordt het bericht via de TCP-verbinding verstuurd.
 *
 * Het bericht wordt ook op de seriële monitor weergegeven voor debugging.
 *
 * @param message Het bericht dat naar de client verstuurd moet worden.
 */
void WifiServerManager::sendMessage(String message) {
    if (client && client.connected()) {
        client.println(message);
    }

    Serial.println(message);
}

/*!
 * @brief Leest een bericht van de verbonden client.
 *
 * Deze functie controleert of er een actieve client is en of er data beschikbaar
 * is. Als er data beschikbaar is, wordt het bericht gelezen tot aan het newline-
 * teken. Daarna wordt witruimte verwijderd en wordt het bericht weergegeven
 * op de seriële monitor.
 *
 * Als er geen bericht beschikbaar is, wordt een lege String teruggegeven.
 *
 * @return Het ontvangen bericht als String, of een lege String als er geen
 * bericht beschikbaar is.
 */
String WifiServerManager::readMessage()
{
    if (client && client.available())
    {
        String message = client.readStringUntil('\n');
        message.trim();

        Serial.print("Ontvangen: ");
        Serial.println(message);

        return message;
    }

    return "";
}