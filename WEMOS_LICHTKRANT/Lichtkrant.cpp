/*!
 * @file Lichtkrant.cpp
 * @brief Implementatie van de Lichtkrant-klasse.
 *
 * Dit bestand bevat de functies voor het aansturen van de MAX7219 LED-matrix.
 * De klasse gebruikt de MD_Parola-library om tekst te tonen en te laten
 * scrollen over de lichtkrant.
 *
 * Binnen het PES restaurantproject wordt deze module gebruikt om berichten
 * zichtbaar te maken op een 32x8 LED-matrix.
 *
 * @author Haki Abdulovski
 * @date 2026
 */

#include "Lichtkrant.h"

/*!
 * @brief Constructor van de Lichtkrant-klasse.
 *
 * Deze constructor maakt het displayobject aan met het opgegeven hardwaretype,
 * de chip-select pin en het aantal gekoppelde MAX7219-modules. Daarnaast wordt
 * een standaard scrollsnelheid ingesteld, wordt de displaystatus op actief
 * gezet en wordt een standaard welkomsttekst opgeslagen.
 *
 * @param hardware Het type MAX7219-hardware dat gebruikt wordt.
 * @param csPin De chip-select pin voor SPI-communicatie.
 * @param aantalDevices Het aantal gekoppelde MAX7219-modules.
 */
Lichtkrant::Lichtkrant(MD_MAX72XX::moduleType_t hardware, int csPin, int aantalDevices)
    : display(hardware, csPin, aantalDevices)
{
    scrollSpeed = 100;
    displayAan = true;
    strcpy(tekst, "WELKOM BIJ DE LICHTKRANT");
}

/*!
 * @brief Initialiseert de MAX7219 LED-matrix.
 *
 * Deze functie start de display, stelt de helderheid in, wist eventuele oude
 * inhoud en toont daarna de standaardtekst.
 */
void Lichtkrant::begin() {
    display.begin();
    display.setIntensity(5);
    display.displayClear();

    toonTekst();
}

/*!
 * @brief Zet de actuele tekst op de display.
 *
 * Deze hulpfunctie wist de display en start een scrollanimatie met de tekst
 * die in de interne tekstbuffer staat. De tekst wordt gecentreerd en scrollt
 * naar links met de ingestelde scrollsnelheid.
 */
void Lichtkrant::toonTekst() {
    display.displayClear();
    display.displayScroll(tekst,
                          PA_CENTER,
                          PA_SCROLL_LEFT,
                          scrollSpeed);
    display.displayReset();
}

/*!
 * @brief Stelt een nieuwe tekst in voor de lichtkrant.
 *
 * Deze functie schakelt de display weer aan, zet de ontvangen String om naar
 * een C-string en slaat deze op in de interne tekstbuffer. Daarna wordt de
 * tekst direct opnieuw op de display gestart.
 *
 * @param nieuweTekst De nieuwe tekst die op de lichtkrant moet verschijnen.
 */
void Lichtkrant::setTekst(String nieuweTekst) {
    displayAan = true;

    nieuweTekst.toCharArray(tekst, sizeof(tekst));

    toonTekst();
}

/*!
 * @brief Past de scrollsnelheid van de lichtkrant aan.
 *
 * Deze functie slaat de nieuwe snelheid op en toont deze via de seriële monitor.
 * Als de display actief is, wordt de tekst opnieuw gestart met de nieuwe
 * scrollsnelheid.
 *
 * @param snelheid De nieuwe scrollsnelheid.
 */
void Lichtkrant::setSnelheid(int snelheid) {
    scrollSpeed = snelheid;

    Serial.print("Nieuwe snelheid: ");
    Serial.println(scrollSpeed);

    if (displayAan) {
        toonTekst();
    }
}

/*!
 * @brief Zet de lichtkrant uit en maakt de display leeg.
 *
 * Deze functie zet de interne displaystatus op uit en wist de LED-matrix.
 * Hierdoor wordt er geen tekst meer weergegeven.
 */
void Lichtkrant::uitzetten() {
    displayAan = false;
    display.displayClear();
    display.displayShutdown(false); // display legen
}

/*!
 * @brief Werkt de scrollanimatie van de lichtkrant bij.
 *
 * Deze functie controleert of de display actief is en of de animatie een stap
 * verder kan. Als de huidige animatie klaar is, wordt de display opnieuw
 * gereset zodat de tekst blijft scrollen.
 */
void Lichtkrant::update() {
    if (displayAan && display.displayAnimate()) {
        display.displayReset();
    }
}