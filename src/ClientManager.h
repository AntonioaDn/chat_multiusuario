#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include <map>
#include <mutex>
#include <memory>
#include <string>
#include <iostream>

// Forward declaration da ClientSession para evitar dependência circular
class ClientSession; 

// Estrutura para manter o estado do cliente
struct ClientInfo {
    int socket_fd;
    std::string username;
    // Opcional: Adicionar um ponteiro fraco (weak_ptr) para a sessão, se necessário
};

class ClientManager {
private:
    // O mapa de clientes é a estrutura crítica, protegida pelo mutex
    std::map<int, std::shared_ptr<ClientSession>> sessions_; //
    std::mutex list_mutex_;

public:
    // Adiciona um novo cliente à lista
    void addClient(std::shared_ptr<ClientSession> session);

    // Remove um cliente da lista (após desconexão)
    void removeClient(int socket_fd);

    // Envia uma mensagem de broadcast para todos os clientes, exceto o remetente
    void broadcastMessage(int sender_fd, const std::string& message);
    
    // Retorna o nome de usuário associado a um socket
    std::string getUsername(int socket_fd);
    
    // (Opcional para a Etapa 3) Retorna o número de clientes ativos
    size_t getActiveCount() {
    std::lock_guard<std::mutex> lock(list_mutex_);
    
    // CORREÇÃO: Usar sessions_ em vez de clients_
    return sessions_.size(); 
}
};

#endif // CLIENT_MANAGER_H