/*!
 * @file AirQualitySensor.cpp
 * @brief Implementatie van de AirQualitySensor-klasse.
 *
 * Dit bestand bevat de functies voor het initialiseren en uitlezen van de
 * SGP30-luchtkwaliteitssensor. De sensor meet eCO2 en TVOC waarden en slaat
 * deze intern op, zodat andere delen van het programma deze waarden kunnen
 * opvragen.
 *
 * Deze klasse wordt gebruikt binnen de CO2-ventilatiemodule van het
 * PES restaurantproject.
 *
 * @author Arbër Deda
 * @date 2026
 */

#include "AirQualitySensor.h"

/*!
 * @brief Initialiseert de I2C-bus en de SGP30-sensor.
 *
 * Deze functie start de I2C-communicatie op de Wemos D1 Mini met SDA op D2
 * en SCL op D1. Daarna wordt gecontroleerd of de SGP30-sensor correct gevonden
 * en gestart kan worden.
 *
 * @return true als de SGP30-sensor succesvol gestart is, false als de sensor
 * niet gevonden of niet correct geïnitialiseerd kan worden.
 */
bool AirQualitySensor::begin()
{
    Wire.begin(D2, D1);

    return sgp.begin();
}

/*!
 * @brief Werkt de gemeten luchtkwaliteitswaarden bij.
 *
 * Deze functie voert een IAQ-meting uit met de SGP30-sensor. Wanneer de meting
 * succesvol is, worden de actuele eCO2- en TVOC-waarden opgeslagen in de
 * interne variabelen van de klasse.
 *
 * @return Geen returnwaarde.
 */
void AirQualitySensor::update()
{
    if(sgp.IAQmeasure())
    {
        eco2 = sgp.eCO2;
        tvoc = sgp.TVOC;
    }
}

/*!
 * @brief Geeft de laatst gemeten eCO2-waarde terug.
 *
 * De eCO2-waarde wordt gebruikt om te bepalen of de ventilatie aan of uit
 * moet worden gezet binnen het restaurant/keukengedeelte.
 *
 * @return De laatst gemeten eCO2-waarde in ppm.
 */
uint16_t AirQualitySensor::getCO2()
{
    return eco2;
}

/*!
 * @brief Geeft de laatst gemeten TVOC-waarde terug.
 *
 * De TVOC-waarde geeft een indicatie van vluchtige organische stoffen in de
 * lucht. Deze waarde wordt opgeslagen zodat deze eventueel gebruikt kan worden
 * voor monitoring of uitbreiding van het systeem.
 *
 * @return De laatst gemeten TVOC-waarde.
 */
uint16_t AirQualitySensor::getTVOC()
{
    return tvoc;
}