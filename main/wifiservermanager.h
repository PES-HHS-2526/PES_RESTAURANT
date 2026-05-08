#ifndef WIFISERVERMANAGER_H
#define WIFISERVERMANAGER_H

#include <ESP8266WiFi.h>

class WifiServerManager {
private:
    const char* ssid;
    const char* password;

    WiFiServer server;
    WiFiClient client;

public:
    WifiServerManager(const char* s, const char* p);

    void begin();

    void checkClient();

    void sendMessage(String message);
};

#endif