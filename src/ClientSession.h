#ifndef CLIENT_SESSION_H
#define CLIENT_SESSION_H

#include <thread>
#include <string>
#include <memory>
#include "libtslog/tslog.h" // Integração com a libtslog

// Forward declaration das classes que a ClientSession usa
class ClientManager;

class ClientSession : public std::enable_shared_from_this<ClientSession> {
private:
    int client_socket_fd_;
    std::string username_;
    std::thread worker_thread_;
    
    // Ponteiros para as estruturas compartilhadas globais
    std::shared_ptr<ClientManager> manager_; 

    // O loop principal de recebimento de mensagens
    void run(); 

public:
    // Construtor
    ClientSession(int socket_fd, std::shared_ptr<ClientManager> manager);

    // Inicia a thread (worker_thread_) para executar run()
    void start();

    // Espera a thread terminar
    void join(); 

    // Envia uma mensagem específica para este cliente
    void sendMessage(const std::string& message);
    
    // Getter para o socket
    int getSocket() const { return client_socket_fd_; }

    // Destrutor (garante que o socket seja fechado e a thread juntada)
    ~ClientSession();
};

#endif // CLIENT_SESSION_H