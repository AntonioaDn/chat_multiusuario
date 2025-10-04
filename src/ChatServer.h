#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <memory>
#include <string>
#include <thread> // NECESSÁRIO
#include <vector>
#include "../libtslog/tslog.h" 

#include "ClientManager.h"
#include "ClientSession.h"
// ... (outros headers de arquitetura)

class ChatServer {
private:
    int server_socket_fd_ = -1;
    int port_;
    
    std::shared_ptr<ClientManager> client_manager_;
    
    std::thread acceptor_thread_; // <--- CORREÇÃO 2: std::thread agora funciona

    void startAcceptLoop();

public:
    ChatServer(int port);
    void start();
    void stop();
    ~ChatServer();
};

#endif // CHAT_SERVER_H