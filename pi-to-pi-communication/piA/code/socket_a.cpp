#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char bericht[1024];
    char buffer[1024];

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "192.168.10.2", &serv_addr.sin_addr);

    while (true) {
        std::cout << "Typ je bericht (of 'exit' om te stoppen): ";
        std::cin.getline(bericht, 1024);

        if (strcmp(bericht, "exit") == 0) break;

        sock = socket(AF_INET, SOCK_STREAM, 0);
        connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

        send(sock, bericht, strlen(bericht), 0);

        memset(buffer, 0, sizeof(buffer));
        read(sock, buffer, 1024);
        std::cout << "Antwoord van server: " << buffer << std::endl;

        close(sock);
    }

    return 0;
}