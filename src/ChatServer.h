#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <memory>
#include <string>
#include <vector>
#include "libtslog/tslog.h" // Logging

// Headers da arquitetura
#include "ClientManager.h"
#include "ClientSession.h"
// ... (Sockets e outras bibliotecas de rede)

class ChatServer {
private:
    int server_socket_fd_;
    int port_;
    
    // O ClientManager é a estrutura central de dados compartilhada
    std::shared_ptr<ClientManager> client_manager_;
    
    // Thread para aceitar novas conexões
    std::thread acceptor_thread_; 

    // Função que executa o loop de aceitar conexões
    void startAcceptLoop();

public:
    ChatServer(int port);

    // Inicia o servidor e o loop de aceitação
    void start();

    // Para o servidor (requer implementação de tratamento de threads/sockets)
    void stop();

    // Destrutor (limpeza de recursos)
    ~ChatServer();
};

#endif // CHAT_SERVER_H