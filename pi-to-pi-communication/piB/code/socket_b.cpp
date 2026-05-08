#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);

    std::cout << "Server wacht op verbinding..." << std::endl;

    while (true) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        memset(buffer, 0, sizeof(buffer));

        read(new_socket, buffer, 1024);
        std::cout << "Bericht ontvangen: " << buffer << std::endl;

        if (strcmp(buffer, "1") == 0) {
            std::cout << "Ontvangen bericht is 1! Extra actie uitgevoerd." << std::endl;
        }

        const char* reply;
        if (strcmp(buffer, "1") == 0) {
            reply = "Led succesvol aan";
        } else {
            reply = "Bericht ontvangen!";
        }
        send(new_socket, reply, strlen(reply), 0);

        close(new_socket);
    }

    close(server_fd);
    return 0;
}