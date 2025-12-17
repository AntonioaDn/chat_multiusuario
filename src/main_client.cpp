#include "ChatClient.h"
#include <iostream>

int main() {
    try {
        ChatClient client;
        // Cliente CLI: conectar 
        client.connectToServer("127.0.0.1", 8080); 
        
        std::cout << "Conectado. Digite mensagens (ou /quit para sair):" << std::endl;
        
        std::string message;
        while (std::getline(std::cin, message)) {
            if (message == "/quit") {
                break;
            }
            // Cliente CLI: enviar mensagens 
            client.sendMessage(message); 
        }
    } catch (const std::exception& e) {
        std::cerr << "Erro fatal no cliente: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}  