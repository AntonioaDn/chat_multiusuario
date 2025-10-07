#include "ClientManager.h"
#include "ClientSession.h"
#include "../libtslog/tslog.h"
#include <unistd.h> // write, close, close
#include <sys/socket.h> // shutdown, SHUT_RDWR
#include <sstream>
#include <vector>

// Adaptação: Agora armazena o shared_ptr para a sessão
void ClientManager::addClient(std::shared_ptr<ClientSession> session) {
    std::lock_guard<std::mutex> lock(list_mutex_); // Exclusão Mútua
    int socket_fd = session->getSocket();
    std::string username = session->getUsername();
    
    sessions_[socket_fd] = session;
    
    TSLOG(INFO, "Cliente " + username + " (socket: " + std::to_string(socket_fd) + ") adicionado. Total: " + std::to_string(sessions_.size()));
}

void ClientManager::removeClient(int socket_fd) {
    std::lock_guard<std::mutex> lock(list_mutex_); // Exclusão Mútua
    auto it = sessions_.find(socket_fd);
    if (it != sessions_.end()) {
        TSLOG(INFO, "Cliente (socket: " + std::to_string(socket_fd) + ") removido. Total: " + std::to_string(sessions_.size() - 1));
        // Fecha o socket do cliente antes de remover a sessão para evitar races
        int fd = it->first;
        try {
            if (fd >= 0) {
                ::shutdown(fd, SHUT_RDWR);
                ::close(fd);
            }
        } catch (...) {
            // não propagar exceções no gerenciador
        }
        sessions_.erase(it);
    }
}

// Funções de formatação e iteração de broadcast
// Broadcast seguro: copia os sessions sob lock e envia fora do lock.
// Se algum send falhar, remove a sessão depois (também protegido por lock).
void ClientManager::broadcastMessage(int from_socket, const std::string &message) {
    std::vector<std::shared_ptr<ClientSession>> sessions_copy;
    std::string sender_name; // Variável para armazenar o nome

    // 1. Aquirir o lock para COPIAR a lista E OBTER o nome do remetente
    {
        std::lock_guard<std::mutex> lg(list_mutex_);

        // Obter o nome do remetente UMA VEZ
        auto it_sender = sessions_.find(from_socket);
        if (it_sender != sessions_.end()) {
            sender_name = it_sender->second->getUsername();
        } else {
            sender_name = "Cliente_" + std::to_string(from_socket);
        }

        // Copiar as sessões dos receptores
        for (const auto &p : sessions_) {
            if (p.first == from_socket) continue;
            sessions_copy.push_back(p.second);
        }
    } // O lock é liberado AQUI (após a chave de fechamento)

    // 2. ENVIAR FORA DO LOCK (I/O)
    std::string formatted_message = sender_name + ": " + message + "\n";
    std::vector<int> to_remove;

    for (auto &sess : sessions_copy) {
        if (!sess) continue;
        // Se o envio falhar (socket fechado), adiciona à lista de remoção
        if (!sess->sendMessage(formatted_message)) {
            to_remove.push_back(sess->getSocket());
        }
    }

    // 3. Aquirir o lock para REMOVER os clientes que falharam
    if (!to_remove.empty()) {
        std::lock_guard<std::mutex> lg(list_mutex_);
        for (int fd : to_remove) {
            sessions_.erase(fd);
            TSLOG(INFO, "Removendo cliente desconectado (socket " + std::to_string(fd) + ")");
        }
    }
}

std::string ClientManager::getUsername(int socket_fd) {
    std::lock_guard<std::mutex> lock(list_mutex_);
    auto it = sessions_.find(socket_fd);
    if (it != sessions_.end()) {
        // Usa o getter da ClientSession
        return it->second->getUsername(); 
    }
    return "UNKNOWN";
}