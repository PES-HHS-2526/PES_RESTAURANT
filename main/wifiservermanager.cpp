#include "WifiServerManager.h"

WifiServerManager::WifiServerManager(const char* s, const char* p) : server(9090)
{
    ssid = s;
    password = p;
}

void WifiServerManager::begin() {
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    server.begin();

    Serial.println("WiFi connected");
    Serial.println(WiFi.localIP());
}

void WifiServerManager::checkClient() {
    if (!client || !client.connected()) {
        client = server.available();
    }
}

void WifiServerManager::sendMessage(String message) {
    if (client && client.connected()) {
        client.println(message);
    }

    Serial.println(message);
}