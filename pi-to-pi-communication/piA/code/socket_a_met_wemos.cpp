#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    const char* wemos_ip = "145.52.127.117";  // IP van Wemos
    int wemos_port = 9090;                    // poort waarop Wemos server draait

    const char* piB_ip = "192.168.10.2";      // IP van Pi B
    int piB_port = 8080;

    char buffer[1024];

    while (true) {
        
        int sockW = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in w_addr;
        w_addr.sin_family = AF_INET;
        w_addr.sin_port = htons(wemos_port);
        inet_pton(AF_INET, wemos_ip, &w_addr.sin_addr);

        if (connect(sockW, (struct sockaddr*)&w_addr, sizeof(w_addr)) < 0) {
            std::cerr << "Kan niet verbinden met Wemos!" << std::endl;
            close(sockW);
            sleep(1);
            continue;
        }

        
        memset(buffer, 0, sizeof(buffer));
        read(sockW, buffer, 1024);
        std::cout << "[Wemos → Pi A] Ontvangen: " << buffer << std::endl;
        close(sockW);

        
        if (strcmp(buffer, "1") == 0) {
            std::cout << "Ontvangen bericht is 1! Wordt doorgestuurd naar Pi B." << std::endl;
        }

        
        int sockB = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in b_addr;
        b_addr.sin_family = AF_INET;
        b_addr.sin_port = htons(piB_port);
        inet_pton(AF_INET, piB_ip, &b_addr.sin_addr);

        if (connect(sockB, (struct sockaddr*)&b_addr, sizeof(b_addr)) < 0) {
            std::cerr << "Kan niet verbinden met Pi B!" << std::endl;
            close(sockB);
            continue;
        }

        
        send(sockB, buffer, strlen(buffer), 0);

        
        memset(buffer, 0, sizeof(buffer));
        read(sockB, buffer, 1024);
        std::cout << "[Pi B → Pi A] Antwoord: " << buffer << std::endl;
        close(sockB);
    }

    return 0;
}