/*!
 * @file button.cpp
 * @brief Implementatie van de Button-klasse.
 *
 * Dit bestand bevat de functies voor het initialiseren en uitlezen van de
 * knop van tafel 2. De knop wordt gebruikt door de klant om een oproep naar
 * de keuken te sturen.
 *
 * @author Arbër Deda
 * @date 2026
 */

#include "Button.h"

/*!
 * @brief Constructor van de Button-klasse.
 *
 * Deze constructor stelt de meegegeven pin in als INPUT_PULLUP. Daardoor is de
 * knopstatus normaal HIGH en wordt deze LOW wanneer de knop wordt ingedrukt.
 * De vorige knopstatus wordt standaard op HIGH gezet.
 *
 * @param p De GPIO-pin waarop de knop is aangesloten.
 */
Button::Button(int p) {
    pin = p;
    pinMode(pin, INPUT_PULLUP);

    lastState = HIGH;
}

/*!
 * @brief Controleert of de knop één keer nieuw is ingedrukt.
 *
 * Deze functie leest de huidige status van de knop. Als de vorige status HIGH
 * was en de huidige status LOW is, betekent dit dat de knop net is ingedrukt.
 * In dat geval wordt true teruggegeven.
 *
 * Na een knopdruk wordt kort gewacht om contactdender te verminderen.
 *
 * @return true als een nieuwe knopdruk is gedetecteerd, anders false.
 */
bool Button::isPressed() {
    bool currentState = digitalRead(pin);

    if (lastState == HIGH && currentState == LOW) {
        lastState = currentState;
        delay(250);
        return true;
    }

    lastState = currentState;
    return false;
}