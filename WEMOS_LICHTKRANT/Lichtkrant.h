/*!
 * @file Lichtkrant.h
 * @brief Declaratie van de Lichtkrant-klasse.
 *
 * Dit headerbestand bevat de klasse die verantwoordelijk is voor het aansturen
 * van de MAX7219 LED-matrix. De klasse gebruikt de MD_Parola-library om tekst
 * te scrollen over de lichtkrant.
 *
 * Binnen het PES restaurantproject wordt deze klasse gebruikt om tekstberichten
 * zichtbaar te maken op een 32x8 LED-matrix.
 *
 * @author Haki Abdulovski
 * @date 2026
 */

#ifndef LICHTKRANT_H
#define LICHTKRANT_H

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

/*!
 * @class Lichtkrant
 * @brief Klasse voor het aansturen van een MAX7219 LED-lichtkrant.
 *
 * De Lichtkrant-klasse beheert de tekstweergave op een MAX7219 LED-matrix.
 * De klasse kan de display initialiseren, tekst laten scrollen, de snelheid
 * aanpassen, de display leegmaken en de animatie bijwerken.
 *
 * Deze klasse vormt de hardware-aansturing van de lichtkrantmodule binnen
 * het PES restaurantproject.
 */
class Lichtkrant {
private:
    /*!
     * @brief Displayobject van de MD_Parola-library.
     *
     * Dit object verzorgt de directe aansturing van de MAX7219 LED-matrix
     * en wordt gebruikt voor tekstweergave en animatie.
     */
    MD_Parola display;

    /*!
     * @brief Buffer waarin de actuele tekst voor de lichtkrant wordt opgeslagen.
     *
     * De tekst wordt als C-string opgeslagen, zodat deze door de MD_Parola-
     * library gebruikt kan worden voor het scrollen op de LED-matrix.
     */
    char tekst[100];

    /*!
     * @brief Scrollsnelheid van de tekst op de lichtkrant.
     *
     * Deze waarde bepaalt hoe snel de tekst over de LED-matrix beweegt.
     */
    int scrollSpeed;

    /*!
     * @brief Geeft aan of de display actief is.
     *
     * Als deze waarde true is, wordt de tekst geanimeerd. Als deze waarde
     * false is, wordt de display leeg gehouden.
     */
    bool displayAan;

    /*!
     * @brief Zet de huidige tekst met de huidige snelheid op de display.
     *
     * Deze hulpfunctie wist eerst de display en start daarna de scrollanimatie
     * opnieuw met de opgeslagen tekst en scrollsnelheid.
     */
    void toonTekst();

public:
    /*!
     * @brief Constructor van de Lichtkrant-klasse.
     *
     * Deze constructor initialiseert het displayobject met het opgegeven
     * hardwaretype, de chip-select pin en het aantal gekoppelde MAX7219-modules.
     *
     * @param hardware Het type MAX7219-hardware dat gebruikt wordt.
     * @param csPin De chip-select pin voor SPI-communicatie.
     * @param aantalDevices Het aantal gekoppelde MAX7219-modules.
     */
    Lichtkrant(MD_MAX72XX::moduleType_t hardware, int csPin, int aantalDevices);

    /*!
     * @brief Initialiseert de LED-matrix.
     *
     * Deze functie start de display, stelt de helderheid in, wist de display
     * en toont de standaardtekst.
     */
    void begin();

    /*!
     * @brief Zet een nieuwe tekst op de lichtkrant.
     *
     * Deze functie schakelt de display aan, slaat de nieuwe tekst op in de
     * interne buffer en start daarna de scrollweergave opnieuw.
     *
     * @param nieuweTekst De tekst die op de lichtkrant weergegeven moet worden.
     */
    void setTekst(String nieuweTekst);

    /*!
     * @brief Past de scrollsnelheid van de lichtkrant aan.
     *
     * Deze functie verandert de snelheid waarmee de tekst over de display
     * beweegt. Als de display actief is, wordt de tekst opnieuw gestart met
     * de nieuwe snelheid.
     *
     * @param snelheid De nieuwe scrollsnelheid.
     */
    void setSnelheid(int snelheid);

    /*!
     * @brief Maakt de display leeg en zet de lichtkrant uit.
     *
     * Deze functie stopt de actieve weergave en wist de LED-matrix.
     */
    void uitzetten();

    /*!
     * @brief Werkt de scrollanimatie van de lichtkrant bij.
     *
     * Deze functie moet continu in de loop worden aangeroepen. Wanneer de
     * display actief is en de animatie klaar is met één scrollstap, wordt de
     * animatie opnieuw gestart.
     */
    void update();
};

#endif