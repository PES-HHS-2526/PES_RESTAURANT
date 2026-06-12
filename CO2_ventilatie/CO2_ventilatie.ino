/*!
 * @file CO2_ventilatie.ino
 * @brief Hoofdprogramma voor de CO2-ventilatiemodule binnen het PES restaurantproject.
 *
 * Dit bestand bevat de hoofdlogica voor de Wemos D1 Mini die gebruikt wordt
 * voor het meten van de luchtkwaliteit in het restaurant/keukengedeelte.
 *
 * De Wemos maakt verbinding met het WiFi-netwerk, leest de CO2/eCO2-waarde uit
 * via de AirQualitySensor-klasse en stuurt via de WifiServerManager-klasse
 * een bericht naar Raspberry Pi-A wanneer de ventilatiestatus verandert.
 *
 * Bij een CO2-waarde hoger dan 500 ppm wordt het bericht "co2:1" verstuurd.
 * Dit betekent dat de ventilatie ingeschakeld moet worden.
 *
 * Bij een CO2-waarde lager dan 450 ppm wordt het bericht "co2:0" verstuurd.
 * Dit betekent dat de ventilatie weer uitgeschakeld mag worden.
 *
 * @author Arbër Deda
 * @date 2026
 */

#include "WifiServerManager.h"
#include "AirQualitySensor.h"

/*!
 * @brief Object voor WiFi- en socketcommunicatie met Raspberry Pi-A.
 *
 * Dit object beheert de verbinding tussen de Wemos D1 Mini en Raspberry Pi-A.
 * De Wemos gebruikt hierbij het opgegeven WiFi-netwerk en wacht op communicatie
 * via de WifiServerManager-klasse.
 */
WifiServerManager wifi(
    "RasberrypiWifi",
    "raspberry1#"
);

/*!
 * @brief Object voor het uitlezen van de luchtkwaliteitssensor.
 *
 * Dit object wordt gebruikt om de SGP30-sensor te initialiseren en de actuele
 * CO2/eCO2-waarde op te vragen tijdens de werking van het programma.
 */
AirQualitySensor sensor;

/*!
 * @brief Houdt bij of de ventilatie momenteel ingeschakeld is.
 *
 * Deze variabele voorkomt dat hetzelfde ventilatiebericht continu opnieuw
 * wordt verstuurd. Alleen wanneer de status verandert, wordt een nieuw bericht
 * naar Raspberry Pi-A gestuurd.
 */
bool ventilationOn = false;

/*!
 * @brief Initialiseert de seriële communicatie, WiFi-verbinding en CO2-sensor.
 *
 * Deze functie wordt één keer uitgevoerd wanneer de Wemos D1 Mini opstart.
 * Eerst wordt de seriële monitor gestart voor debug-informatie. Daarna wordt
 * de WiFi-server gestart en wordt gecontroleerd of de SGP30-sensor gevonden is.
 *
 * Als de sensor niet gevonden wordt, wordt een foutmelding geprint en blijft
 * het programma in een oneindige lus staan. Als de sensor wel gevonden wordt,
 * wordt dit gemeld via de seriële monitor.
 *
 * @return Geen returnwaarde.
 */
void setup()
{
    Serial.begin(115200);

    wifi.begin();

    if(!sensor.begin())
    {
        Serial.println("SGP30 niet gevonden");

        while(true);
    }

    Serial.println("SGP30 gevonden");
}

/*!
 * @brief Hoofdlus van het programma.
 *
 * Deze functie wordt continu herhaald zolang de Wemos D1 Mini actief is.
 * In elke iteratie wordt gecontroleerd of er een client verbonden is, wordt
 * de luchtkwaliteitssensor bijgewerkt en wordt de actuele CO2/eCO2-waarde
 * uitgelezen.
 *
 * Wanneer de CO2-waarde boven de ingestelde bovengrens komt en de ventilatie
 * nog niet aan staat, wordt de ventilatiestatus op aan gezet en wordt het
 * bericht "co2:1" verstuurd.
 *
 * Wanneer de CO2-waarde onder de ingestelde ondergrens komt en de ventilatie
 * nog aan staat, wordt de ventilatiestatus op uit gezet en wordt het bericht
 * "co2:0" verstuurd.
 *
 * Door twee verschillende grenswaarden te gebruiken, 500 ppm voor inschakelen
 * en 450 ppm voor uitschakelen, wordt voorkomen dat de ventilatie snel achter
 * elkaar aan en uit schakelt bij kleine meetverschillen.
 *
 * @return Geen returnwaarde.
 */
void loop()
{
    wifi.checkClient();

    sensor.update();

    uint16_t co2 = sensor.getCO2();

    Serial.print("CO2: ");
    Serial.println(co2);

    if(co2 > 500 && !ventilationOn)
    {
        ventilationOn = true;

        wifi.sendMessage("co2:1");

        Serial.println("Ventilatie AAN");
    }

    else if(co2 < 450 && ventilationOn)
    {
        ventilationOn = false;

        wifi.sendMessage("co2:0");

        Serial.println("Ventilatie UIT");
    }

    delay(1000);
}