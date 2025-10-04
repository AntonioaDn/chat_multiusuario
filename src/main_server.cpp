#include "ChatServer.h"
#include <iostream>

int main() {
    try {
        const int port = 8080;
        ChatServer server(port);
        server.start(); // Bloqueia a thread principal
    } catch (const std::exception& e) {
        std::cerr << "Erro fatal no servidor: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}