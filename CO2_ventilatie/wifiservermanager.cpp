/*!
 * @file WifiServerManager.cpp
 * @brief Implementatie van de WifiServerManager-klasse.
 *
 * Dit bestand bevat de functies voor het opzetten en beheren van de WiFi-
 * en TCP-servercommunicatie op de Wemos D1 Mini. De klasse maakt het mogelijk
 * dat Raspberry Pi-A verbinding maakt met de Wemos en berichten ontvangt of
 * verstuurt.
 *
 * Deze module wordt gebruikt binnen de CO2-ventilatiemodule van het
 * PES restaurantproject.
 *
 * @author Arbër Deda
 * @date 2026
 */

#include "WifiServerManager.h"

/*!
 * @brief Constructor van de WifiServerManager-klasse.
 *
 * Deze constructor slaat de SSID en het wachtwoord van het WiFi-netwerk op
 * en initialiseert de TCP-server op poort 9092.
 *
 * @param s De naam van het WiFi-netwerk waarmee de Wemos verbinding maakt.
 * @param p Het wachtwoord van het WiFi-netwerk.
 */
WifiServerManager::WifiServerManager(const char* s, const char* p) : server(9092)
{
    ssid = s;
    password = p;
}

/*!
 * @brief Start de WiFi-verbinding en de TCP-server.
 *
 * Deze functie stelt eerst de hostname van de Wemos in op "co2sensor".
 * Daarna wordt verbinding gemaakt met het opgegeven WiFi-netwerk.
 * Zodra de WiFi-verbinding actief is, wordt de TCP-server gestart.
 *
 * Het IP-adres van de Wemos wordt via de seriële monitor weergegeven, zodat
 * gecontroleerd kan worden of de module correct verbonden is.
 *
 * @return Geen returnwaarde.
 */
void WifiServerManager::begin() {
    WiFi.hostname("co2sensor");
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
 * Deze functie kijkt of er al een actieve clientverbinding bestaat. Als er geen
 * client verbonden is, wordt gecontroleerd of een nieuwe client verbinding wil
 * maken met de TCP-server.
 *
 * Wanneer een client verbinding maakt, wordt dit gemeld via de seriële monitor.
 *
 * @return Geen returnwaarde.
 */
void WifiServerManager::checkClient() {
    if (!client || !client.connected()) {
        client = server.available();

        if(client)
        {
            Serial.println("Client connected!");
        }
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
 * @param message Het bericht dat naar de client verstuurd moet worden.
 * @return Geen returnwaarde.
 */
void WifiServerManager::sendMessage(String message) {
    if (client && client.connected()) {
        client.println(message);
    }

    Serial.println(message);
}

/*!
 * @brief Leest een ontvangen bericht van de verbonden client.
 *
 * Deze functie controleert of er een client verbonden is en of er data
 * beschikbaar is. Als er data beschikbaar is, wordt het bericht gelezen tot aan
 * een newline-teken. Daarna wordt witruimte verwijderd en wordt het ontvangen
 * bericht weergegeven op de seriële monitor.
 *
 * Als er geen bericht beschikbaar is, wordt een lege String teruggegeven.
 *
 * @return Het ontvangen bericht als String, of een lege String als er geen
 * bericht beschikbaar is.
 */
String WifiServerManager::readMessage() {
    if (client && client.connected() && client.available()) {
        String message = client.readStringUntil('\n');
        message.trim(); // verwijder \r\n witruimte
        Serial.print("Ontvangen: ");
        Serial.println(message);
        return message;
    }

    return "";
}