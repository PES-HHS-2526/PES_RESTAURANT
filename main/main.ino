#include "Button.h"
#include "WifiServerManager.h"

const int buttonPin = D2;
const int ledPin = D1;

Button button(buttonPin);

WifiServerManager wifi(
    "NSELab",
    "NSELabWiFi"
);

bool ledState = false;

void setup() {

    Serial.begin(115200);

    pinMode(ledPin, OUTPUT);

    digitalWrite(ledPin, LOW);

    wifi.begin();
}

void loop() {

    wifi.checkClient();

    if (button.isPressed()) {

        ledState = !ledState;

        if (ledState) {

            wifi.sendMessage("1");

            digitalWrite(ledPin, HIGH);
        }
        else {

            wifi.sendMessage("0");

            digitalWrite(ledPin, LOW);
        }
    }
}