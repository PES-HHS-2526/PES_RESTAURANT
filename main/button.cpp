#include "Button.h"

Button::Button(int p) {
    pin = p;
    pinMode(pin, INPUT_PULLUP);

    lastState = HIGH;
}

bool Button::isPressed() {
    bool currentState = digitalRead(pin);

    if (lastState == HIGH && currentState == LOW) {
        lastState = currentState;
        delay(200);
        return true;
    }

    lastState = currentState;
    return false;
}