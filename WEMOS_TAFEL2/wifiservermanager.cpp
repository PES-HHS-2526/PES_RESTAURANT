/*!
 * @file wifiservermanager.cpp
 * @brief Implementatie van de WifiServerManager-klasse.
 *
 * Dit bestand bevat de functies voor het opzetten en beheren van de WiFi-
 * en TCP-servercommunicatie op de Wemos D1 Mini.
 *
 * Binnen de tafel 2-module wordt deze klasse gebruikt om klantoproepen naar
 * Raspberry Pi-A te sturen en resetberichten terug te ontvangen.
 *
 * @author Arbër Deda
 * @date 2026
 */

#include "WifiServerManager.h"

/*!
 * @brief Constructor van de WifiServerManager-klasse.
 *
 * Deze constructor slaat de SSID en het wachtwoord van het WiFi-netwerk op
 * en initialiseert de TCP-server op poort 9091.
 *
 * @param s De naam van het WiFi-netwerk waarmee de Wemos verbinding maakt.
 * @param p Het wachtwoord van het WiFi-netwerk.
 */
WifiServerManager::WifiServerManager(const char* s, const char* p) : server(9091)
{
    ssid = s;
    password = p;
}

/*!
 * @brief Start de WiFi-verbinding en de TCP-server.
 *
 * Deze functie maakt verbinding met het opgegeven WiFi-netwerk.
 * Zodra de WiFi-verbinding actief is, wordt de TCP-server gestart en wordt het
 * IP-adres van de Wemos weergegeven via de seriële monitor.
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
 * @brief Controleert en beheert de actieve clientverbinding.
 *
 * Als er nog data klaarstaat van de huidige client, wordt de client niet
 * vervangen. Hierdoor wordt voorkomen dat een binnenkomend bericht verloren
 * gaat.
 *
 * Als er geen actieve client is of de vorige client niet meer verbonden is,
 * wordt gecontroleerd of Raspberry Pi-A opnieuw verbinding probeert te maken.
 */
void WifiServerManager::checkClient()
{
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