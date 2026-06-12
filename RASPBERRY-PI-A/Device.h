/*!
 * @file Device.h
 * @brief Declaratie van de Device-klasse voor het dashboard op Raspberry Pi-A.
 *
 * Dit headerbestand bevat de Device-klasse. Deze klasse wordt gebruikt binnen
 * het terminalgebaseerde dashboard van Raspberry Pi-A om apparaten binnen het
 * PES restaurantproject bij te houden.
 *
 * Elk apparaat heeft een naam en een IP-adres. Via de online()-functie kan het
 * dashboard controleren of het apparaat bereikbaar is op het netwerk.
 *
 * @author Haki Abdulovski
 * @date 2026
 */

#ifndef DEVICE_H
#define DEVICE_H

#include <string>

/*!
 * @class Device
 * @brief Klasse voor het opslaan en controleren van een apparaat in het dashboard.
 *
 * De Device-klasse vertegenwoordigt één apparaat binnen het restaurantsysteem.
 * Dit kan bijvoorbeeld een Wemos D1 Mini, Raspberry Pi of ander netwerkapparaat zijn.
 *
 * De klasse bewaart de naam en het IP-adres van het apparaat. Daarnaast bevat
 * de klasse een functie om te controleren of het apparaat online bereikbaar is.
 */
class Device
{
private:
    /*!
     * @brief Naam van het apparaat.
     *
     * Deze naam wordt gebruikt in het dashboard om het apparaat herkenbaar weer
     * te geven.
     */
    std::string name;

    /*!
     * @brief IP-adres van het apparaat.
     *
     * Dit IP-adres wordt gebruikt om de online status van het apparaat te
     * controleren.
     */
    std::string ip;

public:
    /*!
     * @brief Constructor van de Device-klasse.
     *
     * Deze constructor slaat de naam en het IP-adres van het apparaat op.
     *
     * @param n Naam van het apparaat.
     * @param i IP-adres van het apparaat.
     */
    Device(const std::string& n, const std::string& i);

    /*!
     * @brief Controleert of het apparaat online is.
     *
     * Deze functie wordt gebruikt door het dashboard om te bepalen of het
     * apparaat bereikbaar is op het netwerk.
     *
     * @return true als het apparaat online is, anders false.
     */
    bool online();

    /*!
     * @brief Geeft de naam van het apparaat terug.
     *
     * @return Naam van het apparaat.
     */
    std::string getName();

    /*!
     * @brief Geeft het IP-adres van het apparaat terug.
     *
     * @return IP-adres van het apparaat.
     */
    std::string getIp();
};

#endif