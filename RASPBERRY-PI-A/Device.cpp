/*!
 * @file Device.cpp
 * @brief Implementatie van de Device-klasse voor het dashboard op Raspberry Pi-A.
 *
 * Dit bestand bevat de implementatie van de Device-klasse. Deze klasse wordt
 * gebruikt binnen het terminalgebaseerde dashboard om apparaten binnen het PES
 * restaurantproject weer te geven en hun online status te controleren.
 *
 * De online status wordt gecontroleerd met een ping-commando naar het ingestelde
 * IP-adres van het apparaat.
 *
 * @author Haki Abdulovski
 * @date 2026
 */

#include "Device.h"

#include <cstdlib>

/*!
 * @brief Constructor van de Device-klasse.
 *
 * Deze constructor slaat de naam en het IP-adres van het apparaat op. Deze
 * gegevens worden later gebruikt door het dashboard om apparaten te tonen en
 * hun bereikbaarheid te controleren.
 *
 * @param n Naam van het apparaat.
 * @param i IP-adres van het apparaat.
 */
Device::Device(const std::string& n, const std::string& i)
{
    name = n;
    ip = i;
}

/*!
 * @brief Controleert of het apparaat online is.
 *
 * Deze functie voert een ping-commando uit naar het IP-adres van het apparaat.
 * Er wordt één ping verstuurd met een korte timeout. De uitvoer van het commando
 * wordt verborgen, zodat alleen het resultaat wordt gebruikt.
 *
 * Als het ping-commando succesvol is, wordt het apparaat als online gezien.
 *
 * @return true als het apparaat bereikbaar is, anders false.
 */
bool Device::online()
{
    std::string command = "ping -c 1 -W 1 " + ip + " > /dev/null 2>&1";

    return system(command.c_str()) == 0;
}

/*!
 * @brief Geeft de naam van het apparaat terug.
 *
 * Deze functie wordt gebruikt door het dashboard om de apparaatnaam weer te geven.
 *
 * @return Naam van het apparaat.
 */
std::string Device::getName()
{
    return name;
}

/*!
 * @brief Geeft het IP-adres van het apparaat terug.
 *
 * Deze functie wordt gebruikt door het dashboard om het IP-adres van het apparaat
 * weer te geven.
 *
 * @return IP-adres van het apparaat.
 */
std::string Device::getIp()
{
    return ip;
}