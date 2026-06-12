/*!
 * @file button.h
 * @brief Declaratie van de Button-klasse voor het uitlezen van een drukknop.
 *
 * Dit headerbestand bevat de klasse die verantwoordelijk is voor het uitlezen
 * van de fysieke knop van tafel 2. De klasse detecteert een nieuwe knopdruk
 * en voorkomt dat één ingedrukte knop meerdere keren direct wordt geregistreerd.
 *
 * @author Arbër Deda
 * @date 2026
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

/*!
 * @class Button
 * @brief Klasse voor het uitlezen van een knop met interne pull-up.
 *
 * De Button-klasse beheert één digitale invoerpin. De knop wordt aangesloten
 * met INPUT_PULLUP, waardoor de pin normaal HIGH is en LOW wordt wanneer de
 * knop wordt ingedrukt.
 *
 * De klasse onthoudt de vorige status van de knop, zodat alleen een nieuwe
 * overgang van niet-ingedrukt naar ingedrukt als knopdruk wordt gezien.
 */
class Button {
private:
    /*!
     * @brief GPIO-pin waarop de knop is aangesloten.
     */
    int pin;

    /*!
     * @brief Vorige status van de knop.
     *
     * Deze waarde wordt gebruikt om te bepalen of er een nieuwe knopdruk heeft
     * plaatsgevonden.
     */
    bool lastState;

public:
    /*!
     * @brief Constructor van de Button-klasse.
     *
     * Deze constructor slaat de gebruikte pin op en stelt deze pin in als
     * INPUT_PULLUP.
     *
     * @param p De GPIO-pin waarop de knop is aangesloten.
     */
    Button(int p);

    /*!
     * @brief Controleert of de knop opnieuw is ingedrukt.
     *
     * Deze functie leest de actuele knopstatus en vergelijkt deze met de vorige
     * status. Alleen wanneer de knop van HIGH naar LOW gaat, wordt true
     * teruggegeven.
     *
     * @return true als er een nieuwe knopdruk is gedetecteerd, anders false.
     */
    bool isPressed();
};

#endif