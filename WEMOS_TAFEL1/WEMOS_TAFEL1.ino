/*!
 * @file WEMOS_TAFEL1.ino
 * @brief Hoofdprogramma voor de Wemos D1 Mini van tafel 1 binnen het PES restaurantproject.
 *
 * Dit bestand bevat de hoofdlogica voor de klantoproepmodule van tafel 1.
 * Wanneer een klant op de knop drukt, gaat de LED bij de tafel aan en wordt
 * via WiFi/TCP het bericht "tafel1:1" naar Raspberry Pi-A gestuurd.
 *
 * Raspberry Pi-A stuurt deze melding door naar Raspberry Pi-B. Raspberry Pi-B
 * stuurt vervolgens via CANBUS een bericht naar een STM32-module in de keuken,
 * zodat zichtbaar wordt dat tafel 1 hulp nodig heeft of een bestelling klaarstaat.
 *
 * Wanneer de serveerster in de keuken op de resetknop drukt, wordt via de keten
 * STM32 -> Raspberry Pi-B -> Raspberry Pi-A -> Wemos het bericht "tafel1:0"
 * teruggestuurd. De Wemos zet daarna de LED bij de klant uit.
 *
 * @author Arbër Deda
 * @date 2026
 */

#include "Button.h"
#include "WifiServerManager.h"

/*!
 * @brief GPIO-pin waarop de knop van tafel 1 is aangesloten.
 *
 * Deze pin wordt gebruikt om te detecteren of de klant op de oproepknop drukt.
 */
const int buttonPin = D2;

/*!
 * @brief GPIO-pin waarop de LED van tafel 1 is aangesloten.
 *
 * Deze LED geeft bij de klant aan of de oproep actief is.
 */
const int ledPin = D1;

/*!
 * @brief Button-object voor het uitlezen van de klantknop.
 *
 * Dit object gebruikt de Button-klasse om een druk op de knop te detecteren.
 */
Button button(buttonPin);

/*!
 * @brief Object voor WiFi- en TCP-servercommunicatie met Raspberry Pi-A.
 *
 * De Wemos maakt verbinding met het opgegeven WiFi-netwerk en wacht als
 * TCP-server op verbinding van Raspberry Pi-A.
 */
WifiServerManager wifi(
    "RasberrypiWifi",
    "raspberry1#"
);

/*!
 * @brief Houdt bij of de tafel-LED momenteel aan staat.
 *
 * Deze variabele voorkomt dat het bericht "tafel1:1" herhaaldelijk wordt
 * verstuurd zolang de oproep al actief is.
 */
bool ledState = false;

/*!
 * @brief Initialiseert de seriële communicatie, LED-pin en WiFi-server.
 *
 * Deze functie wordt één keer uitgevoerd wanneer de Wemos D1 Mini opstart.
 * De LED wordt als output ingesteld en standaard uitgezet. Daarna wordt de
 * WiFi-server gestart zodat Raspberry Pi-A verbinding kan maken.
 */
void setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    wifi.begin();
}

/*!
 * @brief Hoofdlus voor de klantoproep van tafel 1.
 *
 * Deze functie wordt continu uitgevoerd. Eerst wordt gecontroleerd of er een
 * Raspberry Pi-client verbonden is en of er een bericht ontvangen is.
 *
 * Wanneer de klant op de knop drukt en de oproep nog niet actief is, wordt
 * de tafel-LED aangezet en wordt het bericht "tafel1:1" naar Raspberry Pi-A
 * gestuurd.
 *
 * Wanneer het bericht "tafel1:0" wordt ontvangen, betekent dit dat de oproep
 * door de keuken of serveerster is afgehandeld. De LED bij de tafel wordt dan
 * uitgezet.
 */
void loop() {
    wifi.checkClient();

String incoming = wifi.readMessage();

if (incoming != "") {
    Serial.print("RAW:");
    Serial.println(incoming);
}
    if (button.isPressed() && !ledState) {
        ledState = true;
        wifi.sendMessage("tafel1:1");
        digitalWrite(ledPin, HIGH);
    }

    if (incoming == "tafel1:0") {
        ledState = false;
        wifi.sendMessage("tafel1:0");
        digitalWrite(ledPin, LOW);
    }
}