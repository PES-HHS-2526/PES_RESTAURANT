/*!
 * @file AirQualitySensor.h
 * @brief Declaratie van de AirQualitySensor-klasse.
 *
 * Dit headerbestand bevat de klasse die verantwoordelijk is voor het aansturen
 * en uitlezen van de SGP30-luchtkwaliteitssensor. De klasse maakt gebruik van
 * de Adafruit_SGP30-library en levert functies om eCO2- en TVOC-waarden op te
 * vragen.
 *
 * Deze klasse wordt gebruikt door het hoofdprogramma van de CO2-ventilatiemodule.
 *
 * @author Arbër Deda
 * @date 2026
 */

#ifndef AIRQUALITYSENSOR_H
#define AIRQUALITYSENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SGP30.h"

/*!
 * @class AirQualitySensor
 * @brief Klasse voor het initialiseren en uitlezen van de SGP30-luchtkwaliteitssensor.
 *
 * De AirQualitySensor-klasse beheert de communicatie met de SGP30-sensor.
 * De klasse initialiseert de sensor via I2C, voert metingen uit en slaat de
 * laatst gemeten eCO2- en TVOC-waarden op.
 *
 * Binnen het PES restaurantproject wordt vooral de eCO2-waarde gebruikt om
 * te bepalen of de ventilatie ingeschakeld of uitgeschakeld moet worden.
 */
class AirQualitySensor
{
private:
    /*!
     * @brief Object van de Adafruit SGP30-library.
     *
     * Dit object verzorgt de directe communicatie met de fysieke SGP30-sensor.
     */
    Adafruit_SGP30 sgp;

    /*!
     * @brief Laatst gemeten eCO2-waarde.
     *
     * Deze waarde wordt gebruikt als CO2-indicatie voor de ventilatieregeling.
     */
    uint16_t eco2;

    /*!
     * @brief Laatst gemeten TVOC-waarde.
     *
     * Deze waarde geeft een indicatie van vluchtige organische stoffen in de lucht.
     */
    uint16_t tvoc;

public:
    /*!
     * @brief Initialiseert de I2C-verbinding en de SGP30-sensor.
     *
     * @return true als de sensor correct gestart is, false als initialisatie mislukt.
     */
    bool begin();

    /*!
     * @brief Voert een nieuwe meting uit en werkt de interne sensorwaarden bij.
     *
     * @return Geen returnwaarde.
     */
    void update();

    /*!
     * @brief Geeft de laatst opgeslagen eCO2-waarde terug.
     *
     * @return De laatst gemeten eCO2-waarde in ppm.
     */
    uint16_t getCO2();

    /*!
     * @brief Geeft de laatst opgeslagen TVOC-waarde terug.
     *
     * @return De laatst gemeten TVOC-waarde.
     */
    uint16_t getTVOC();
};

#endif