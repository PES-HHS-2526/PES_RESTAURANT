/*!
 * @file PiCommunicator.cpp
 * @brief Implementatie van de PiCommunicator-klasse voor Raspberry Pi-A.
 *
 * Dit bestand bevat de centrale communicatieafhandeling van Raspberry Pi-A.
 * Raspberry Pi-A maakt verbinding met meerdere Wemos D1 Mini-modules en met
 * Raspberry Pi-B.
 *
 * Berichten van Wemos-modules, zoals tafeloproepen, CO2/eCO2-statussen en
 * RFID-statussen, worden door Raspberry Pi-A ontvangen en doorgestuurd naar
 * Raspberry Pi-B. Raspberry Pi-B zet deze berichten daarna om naar CANBUS-
 * berichten voor de STM32-microcontrollers.
 *
 * Terugmeldingen vanaf Raspberry Pi-B, zoals resetberichten voor tafel 1 of
 * tafel 2, worden door Raspberry Pi-A teruggestuurd naar de juiste Wemos-module.
 *
 * @author Haki Abdulovski
 * @date 2026
 */

#include "PiCommunicator.h"
#include "SocketClient.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <vector>
#include <algorithm>

/*!
 * @brief Constructor van de PiCommunicator-klasse.
 *
 * Deze constructor slaat de lijst met Wemos-modules en de verbindingsgegevens
 * van Raspberry Pi-B op. De daadwerkelijke verbindingen worden later gemaakt
 * in runLoop().
 *
 * @param wemosList Lijst met WemosDevice-objecten waarmee Raspberry Pi-A moet communiceren.
 * @param b_ip IP-adres van Raspberry Pi-B.
 * @param b_port TCP-poort van Raspberry Pi-B.
 */
PiCommunicator::PiCommunicator(const std::vector<WemosDevice>& wemosList,
                               const std::string& b_ip, int b_port)
    : wemosDevices(wemosList), piBIP(b_ip), piBPort(b_port) {}

/*!
 * @brief Anonieme namespace voor hulpfuncties binnen dit bestand.
 *
 * De functies in deze namespace zijn alleen beschikbaar binnen dit cpp-bestand.
 * Ze worden gebruikt om berichten leesbaarder te maken in de terminaluitvoer.
 */
namespace {
    /*!
     * @brief Zet een string om naar kleine letters.
     *
     * Deze functie wordt gebruikt om berichtidentificaties hoofdletterongevoelig
     * te kunnen vergelijken, bijvoorbeeld "RFID" en "rfid".
     *
     * @param s De string die omgezet moet worden.
     * @return De omgezette string in kleine letters.
     */
    std::string toLower(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

    /*!
     * @brief Maakt een leesbare omschrijving van een ontvangen bericht.
     *
     * Deze functie zet technische berichten zoals "tafel1:1", "co2:0" of
     * "RFID:1" om naar een duidelijkere tekst voor de terminaluitvoer.
     *
     * @param id Identificatie van het bericht, bijvoorbeeld "tafel1", "co2" of "RFID".
     * @param value Waarde van het bericht, bijvoorbeeld "1" of "0".
     * @return Leesbare omschrijving van het bericht.
     */
    std::string omschrijfBericht(const std::string& id, const std::string& value)
    {
        const std::string key = toLower(id);

        if (key.rfind("tafel", 0) == 0)         
        {
            if (value == "1") return id + " knop ingedrukt";
            if (value == "0") return id + " reset";
        }
        else if (key == "co2")
        {
            if (value == "1") return "CO2 te hoog";
            if (value == "0") return "CO2 weer normaal";
        }
        else if (key == "rfid")
        {
            if (value == "1") return "RFID toegang verleend";
            if (value == "0") return "RFID toegang geweigerd";
        }
        return "onbekend bericht '" + id + ":" + value + "'";
    }
}

/*!
 * @brief Parseert een bericht met dubbele punt naar identificatie en waarde.
 *
 * Deze functie verwerkt berichten in de vorm "tafel1:1", "tafel2:0",
 * "co2:1" of "RFID:1". Alles vóór de dubbele punt wordt opgeslagen als
 * identificatie. Alles na de dubbele punt wordt opgeslagen als waarde.
 *
 * @param msg Het bericht dat geparsed moet worden.
 * @param table Referentie waarin de identificatie wordt opgeslagen.
 * @param value Referentie waarin de waarde wordt opgeslagen.
 * @return true als het bericht succesvol geparsed is, anders false.
 */
bool PiCommunicator::parseTableMessage(const std::string& msg,
                                       std::string& table, std::string& value)
{
    size_t pos = msg.find(':');
    if (pos == std::string::npos) return false;
    table = msg.substr(0, pos);
    value = msg.substr(pos + 1);
    return !table.empty() && !value.empty();
}

/*!
 * @brief Verwerkt een bericht dat vanaf een Wemos-module binnenkomt.
 *
 * Deze functie ontvangt een bericht van een specifieke Wemos-module. Als het
 * bericht al een volledige vorm heeft, zoals "tafel1:1" of "co2:0", wordt het
 * direct doorgestuurd.
 *
 * Als het bericht alleen een losse waarde bevat, wordt de identificatie van de
 * Wemos-module ervoor geplaatst. Zo wordt bijvoorbeeld waarde "1" van tafel1
 * omgezet naar "tafel1:1".
 *
 * Het genormaliseerde bericht wordt daarna doorgestuurd naar Raspberry Pi-B.
 *
 * @param dev De Wemos-module waarvan het bericht afkomstig is.
 * @param msg Het ontvangen bericht.
 */
void PiCommunicator::handleMessageFromWemos(const WemosDevice& dev, const std::string& msg)
{

    std::string table, value, normalized;

    if (parseTableMessage(msg, table, value))
    {
        normalized = msg;
    }
    else
    {
        table = dev.tableId;
        value = msg;
        normalized = dev.tableId + ":" + msg;
    }

    std::cout << "[Pi A] " << omschrijfBericht(table, value)
              << ", doorsturen naar Pi B." << std::endl;

    if (piBClient)
        piBClient->sendMessage(normalized);
}

/*!
 * @brief Verwerkt een bericht dat vanaf Raspberry Pi-B binnenkomt.
 *
 * Deze functie wordt gebruikt voor terugmeldingen van Raspberry Pi-B. Wanneer
 * een bericht zoals "tafel1:0" of "tafel2:0" binnenkomt, wordt het bericht
 * geparsed en doorgestuurd naar de juiste Wemos-module.
 *
 * Als alleen "1" of "0" wordt ontvangen, wordt dit als fallback naar de eerste
 * Wemos-module gestuurd.
 *
 * Onbekende berichten worden genegeerd en alleen weergegeven in de terminal.
 *
 * @param msg Het ontvangen bericht vanaf Raspberry Pi-B.
 */
void PiCommunicator::handleMessageFromPiB(const std::string& msg)
{
    std::cout << "[Pi A] Bericht van Pi B: '" << msg << "'" << std::endl;

    std::string table, value;

    if (parseTableMessage(msg, table, value))
    {
        std::cout << "[Pi A] " << omschrijfBericht(table, value) << "." << std::endl;
        sendToWemos(table, value);
    }
    else if (msg == "1" || msg == "0")
    {
        if (!wemosDevices.empty())
            sendToWemos(wemosDevices[0].tableId, msg);
    }
    else
    {
        std::cout << "[Pi A] Bericht van Pi B genegeerd: '" << msg << "'" << std::endl;
    }
}

/*!
 * @brief Stuurt een bericht naar een specifieke Wemos-module.
 *
 * Deze functie zoekt in de lijst met Wemos-modules naar de opgegeven tableId.
 * Wanneer de juiste Wemos gevonden is, wordt het bericht samengesteld in de vorm
 * "tableId:value" en via de bijbehorende SocketClient verzonden.
 *
 * Voorbeelden zijn:
 * - "tafel1:0" om de LED van tafel 1 uit te zetten;
 * - "tafel2:0" om de LED van tafel 2 uit te zetten.
 *
 * @param tableId Identificatie van de Wemos-module.
 * @param value Waarde die naar de Wemos-module gestuurd moet worden.
 */
void PiCommunicator::sendToWemos(const std::string& tableId, const std::string& value)
{
    for (size_t i = 0; i < wemosDevices.size(); ++i)
    {
        if (wemosDevices[i].tableId == tableId)
        {
            std::string fullMessage = tableId + ":" + value;
            std::cout << "[Pi A] Stuur '" << fullMessage << "' naar " << tableId << "." << std::endl;
            if (wemosClients[i])
                wemosClients[i]->sendMessage(fullMessage);
            return;
        }
    }
    std::cerr << "[Pi A] Geen Wemos gevonden voor '" << tableId << "'." << std::endl;
}

/*!
 * @brief Start de communicatie tussen Raspberry Pi-A, Wemos-modules en Raspberry Pi-B.
 *
 * Deze functie controleert eerst of er Wemos-modules zijn ingesteld. Daarna
 * maakt Raspberry Pi-A verbinding met Raspberry Pi-B. Als de verbinding niet
 * lukt, wordt elke twee seconden opnieuw geprobeerd.
 *
 * Vervolgens maakt Raspberry Pi-A verbinding met alle ingestelde Wemos-modules.
 * Voor elke verbinding wordt een SocketClient-object aangemaakt.
 *
 * Na het maken van de verbindingen worden meerdere threads gestart:
 * - één thread luistert naar berichten van Raspberry Pi-B;
 * - per Wemos-module wordt één thread gestart om berichten van die Wemos te lezen.
 *
 * Hierdoor kan Raspberry Pi-A tegelijk berichten ontvangen van meerdere Wemos-
 * modules en Raspberry Pi-B.
 */
void PiCommunicator::runLoop()
{
    if (wemosDevices.empty())
    {
        std::cerr << "Geen Wemos devices ingesteld." << std::endl;
        return;
    }

    piBClient = std::make_shared<SocketClient>(piBIP, piBPort, "Pi B");
    while (!piBClient->connectToServer())
    {
        std::cerr << "Kan niet verbinden met Pi B, opnieuw proberen..." << std::endl;
        sleep(2);
    }
    std::cout << "Verbonden met Pi B!" << std::endl;

    wemosClients.resize(wemosDevices.size());
    for (size_t i = 0; i < wemosDevices.size(); ++i)
    {
        const WemosDevice& w = wemosDevices[i];
        wemosClients[i] = std::make_shared<SocketClient>(w.ip, w.port, w.tableId);

        while (!wemosClients[i]->connectToServer())
        {
            std::cerr << "Kan niet verbinden met " << w.tableId
                      << " (" << w.ip << "), opnieuw proberen..." << std::endl;
            sleep(2);
        }
        std::cout << "Verbonden met " << w.tableId << " op "
                  << w.ip << ":" << w.port << std::endl;
    }

    std::thread piBThread(&SocketClient::receiveLoop, piBClient.get(),
        [this](const std::string& msg) {
            handleMessageFromPiB(msg);
        });

    std::vector<std::thread> wemosThreads;
    for (size_t i = 0; i < wemosClients.size(); ++i)
    {
        WemosDevice dev   = wemosDevices[i];    
        SocketClient* cli = wemosClients[i].get();

        wemosThreads.emplace_back([this, cli, dev]() {
            cli->receiveLoop([this, dev](const std::string& msg) {
                handleMessageFromWemos(dev, msg);
            });
        });
    }

    piBThread.join();
    for (auto& t : wemosThreads)
        t.join();
}