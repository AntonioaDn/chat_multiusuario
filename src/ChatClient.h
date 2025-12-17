#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include <string>
#include <thread>
#include <unistd.h>
#include "../libtslog/tslog.h" 

class ChatClient {
private:
    int client_socket_fd_ = -1;
    bool connected_ = false;
    std::thread receiver_thread_;

    // Loop que escuta e exibe mensagens do servidor
    void receiverLoop(); 

public:
    ChatClient();

    // Conecta o socket ao IP e porta do servidor
    void connectToServer(const std::string& ip, int port);

    // Envia uma mensagem para o servidor
    void sendMessage(const std::string& message);

    // Fecha o socket e encerra a thread
    void disconnect();

    ~ChatClient();
};

#endif // CHAT_CLIENT_H