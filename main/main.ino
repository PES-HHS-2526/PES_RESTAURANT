#include "Button.h"
#include "WifiServerManager.h"

Button button(D1);

WifiServerManager wifi(
    "NSELab",
    "NSELabWiFi"
);

bool ledState = false;

void setup() {
    Serial.begin(115200);

    wifi.begin();
}

void loop() {
    wifi.checkClient();

    if (button.isPressed()) {

        ledState = !ledState;

        if (ledState) {
            wifi.sendMessage("1");
        }
        else {
            wifi.sendMessage("0");
        }
    }
}