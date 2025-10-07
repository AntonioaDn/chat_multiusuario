#ifndef CLIENT_SESSION_H
#define CLIENT_SESSION_H

#include <thread> // NECESSÁRIO
#include <string>
#include <memory>
#include "../libtslog/tslog.h" 

class ClientManager; // Forward declaration
class MessageHistory;

class ClientSession : public std::enable_shared_from_this<ClientSession> {
private:
    int client_socket_fd_ = -1; // <--- CORREÇÃO 1
    std::string username_;      // <--- CORREÇÃO 1
    std::thread worker_thread_; 
    
    std::shared_ptr<ClientManager> manager_; 
    std::shared_ptr<MessageHistory> history_; 

    void run(); 

public:
    // <--- CORREÇÃO 3: DECLARAÇÃO DO CONSTRUTOR
    ClientSession(int socket_fd, std::shared_ptr<ClientManager> manager, std::shared_ptr<MessageHistory> history);

    void start();
    bool sendMessage(const std::string& message);
    
    // Getters
    int getSocket() const { return client_socket_fd_; } 
    std::string getUsername() const { return username_; }

    ~ClientSession();
};

#endif // CLIENT_SESSION_H