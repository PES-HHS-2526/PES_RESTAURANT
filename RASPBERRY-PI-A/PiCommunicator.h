/*!
 * @file PiCommunicator.h
 * @brief Declaratie van de PiCommunicator-klasse voor communicatie op Raspberry Pi-A.
 *
 * Dit headerbestand bevat de PiCommunicator-klasse. Deze klasse vormt de centrale
 * communicatiebrug op Raspberry Pi-A tussen de Wemos D1 Mini-modules en
 * Raspberry Pi-B.
 *
 * Binnen het PES restaurantproject ontvangt Raspberry Pi-A berichten van
 * verschillende Wemos-modules, zoals tafeloproepen, CO2/eCO2-statussen en
 * RFID-statussen. Deze berichten worden vervolgens doorgestuurd naar Raspberry
 * Pi-B. Raspberry Pi-B zet deze berichten daarna om naar CANBUS-berichten voor
 * de STM32-microcontrollers.
 *
 * Ook verwerkt deze klasse terugmeldingen vanaf Raspberry Pi-B, zoals
 * resetberichten voor tafeloproepen. Deze worden doorgestuurd naar de juiste
 * Wemos-module.
 *
 * @author Haki Abdulovski
 * @date 2026
 */

#ifndef PICOMMUNICATOR_H
#define PICOMMUNICATOR_H

#include <string>
#include <vector>
#include <memory>

/*!
 * @struct WemosDevice
 * @brief Struct voor het opslaan van Wemos-verbindingsgegevens.
 *
 * Deze struct bevat de gegevens die Raspberry Pi-A nodig heeft om verbinding
 * te maken met een specifieke Wemos D1 Mini-module.
 *
 * Elke Wemos-module krijgt een eigen identificatie, IP-adres en poortnummer.
 * Hierdoor kan Raspberry Pi-A berichten koppelen aan de juiste module, zoals
 * tafel 1, tafel 2, CO2 of RFID.
 */
struct WemosDevice {
    /*!
     * @brief Identificatie van de Wemos-module.
     *
     * Voorbeelden zijn "tafel1", "tafel2", "co2" of "rfid".
     */
    std::string tableId; 

    /*!
     * @brief IP-adres van de Wemos D1 Mini-module.
     */
    std::string ip;

    /*!
     * @brief TCP-poort waarop de Wemos D1 Mini luistert.
     */
    int port;
};

/*!
 * @brief Forward declaration van de SocketClient-klasse.
 *
 * De volledige definitie van SocketClient is niet nodig in dit headerbestand.
 * Hierdoor blijft de afhankelijkheid kleiner en wordt de klasse pas in de
 * implementatie gebruikt.
 */
class SocketClient;

/*!
 * @class PiCommunicator
 * @brief Centrale communicatieklasse van Raspberry Pi-A.
 *
 * De PiCommunicator-klasse beheert de communicatie tussen Raspberry Pi-A,
 * meerdere Wemos D1 Mini-modules en Raspberry Pi-B.
 *
 * De klasse maakt verbinding met:
 * - Raspberry Pi-B voor de communicatie richting CANBUS en STM32;
 * - Wemos Tafel 1 voor klantoproepen;
 * - Wemos Tafel 2 voor klantoproepen;
 * - Wemos CO2 voor ventilatiestatus;
 * - Wemos RFID voor toegangscontrole.
 *
 * Berichten vanaf Wemos-modules worden verwerkt en doorgestuurd naar Raspberry
 * Pi-B. Berichten vanaf Raspberry Pi-B worden verwerkt en indien nodig
 * doorgestuurd naar de juiste Wemos-module.
 */
class PiCommunicator {
public:
    /*!
     * @brief Constructor van de PiCommunicator-klasse.
     *
     * Deze constructor slaat de lijst met Wemos-modules en de verbindingsgegevens
     * van Raspberry Pi-B op.
     *
     * @param wemosList Lijst met WemosDevice-objecten die Raspberry Pi-A moet beheren.
     * @param b_ip IP-adres van Raspberry Pi-B.
     * @param b_port TCP-poort van Raspberry Pi-B.
     */
    PiCommunicator(const std::vector<WemosDevice>& wemosList, const std::string& b_ip, int b_port);

    /*!
     * @brief Start de hoofdcommunicatielus van Raspberry Pi-A.
     *
     * Deze functie maakt verbinding met Raspberry Pi-B en de ingestelde
     * Wemos-modules. Daarna worden inkomende berichten continu verwerkt.
     */
    void runLoop();

private:
    /*!
     * @brief Lijst met Wemos-modules waarmee Raspberry Pi-A communiceert.
     */
    std::vector<WemosDevice> wemosDevices;

    /*!
     * @brief IP-adres van Raspberry Pi-B.
     */
    std::string piBIP;

    /*!
     * @brief TCP-poort van Raspberry Pi-B.
     */
    int piBPort;

    /*!
     * @brief SocketClient voor de verbinding met Raspberry Pi-B.
     */
    std::shared_ptr<SocketClient> piBClient;

    /*!
     * @brief Lijst met SocketClient-objecten voor de Wemos-modules.
     */
    std::vector<std::shared_ptr<SocketClient>> wemosClients;

    /*!
     * @brief Verwerkt een bericht dat afkomstig is van een Wemos-module.
     *
     * Deze functie bepaalt van welke Wemos-module het bericht afkomstig is en
     * stuurt het bericht indien nodig door naar Raspberry Pi-B.
     *
     * @param dev De Wemos-module waarvan het bericht afkomstig is.
     * @param msg Het ontvangen bericht.
     */
    void handleMessageFromWemos(const WemosDevice& dev, const std::string& msg);

    /*!
     * @brief Verwerkt een bericht dat afkomstig is van Raspberry Pi-B.
     *
     * Deze functie wordt gebruikt voor terugmeldingen van Raspberry Pi-B.
     * Voorbeelden hiervan zijn resetberichten zoals "tafel1:0" en "tafel2:0".
     *
     * @param msg Het ontvangen bericht vanaf Raspberry Pi-B.
     */
    void handleMessageFromPiB(const std::string& msg);

    /*!
     * @brief Stuurt een bericht naar een specifieke Wemos-module.
     *
     * Deze functie zoekt op basis van de tableId de juiste Wemos-module en
     * stuurt daar de opgegeven waarde naartoe.
     *
     * @param tableId Identificatie van de Wemos-module.
     * @param value Berichtwaarde die naar de Wemos-module gestuurd moet worden.
     */
    void sendToWemos(const std::string& tableId, const std::string& value);

    /*!
     * @brief Parseert een tafelbericht naar tafelnaam en waarde.
     *
     * Deze functie verwerkt berichten in de vorm "tafel1:0" of "tafel2:1".
     * De tekst vóór de dubbele punt wordt opgeslagen als tafelnaam en de tekst
     * na de dubbele punt als waarde.
     *
     * @param msg Het bericht dat geparsed moet worden.
     * @param table Referentie waarin de tafelidentificatie wordt opgeslagen.
     * @param value Referentie waarin de berichtwaarde wordt opgeslagen.
     * @return true als het bericht succesvol geparsed is, anders false.
     */
    static bool parseTableMessage(const std::string& msg, std::string& table, std::string& value);
};

#endif