#include "ClientManager.h"
#include "ClientSession.h"
#include "../libtslog/tslog.h"
#include <unistd.h> // write, close
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
        sessions_.erase(it);
    }
}

// Funções de formatação e iteração de broadcast
// Broadcast seguro: copia os sessions sob lock e envia fora do lock.
// Se algum send falhar, remove a sessão depois (também protegido por lock).
void ClientManager::broadcastMessage(int from_socket, const std::string &message) {
    // 1) copiar shared_ptrs enquanto segura o mutex
    std::vector<std::shared_ptr<ClientSession>> sessions_copy;
    {
        std::lock_guard<std::mutex> lg(list_mutex_);
        sessions_copy.reserve(sessions_.size());
        for (const auto &p : sessions_) {
            // opcional: não ecoar para o remetente
            if (p.first == from_socket) continue;
            sessions_copy.push_back(p.second);
        }
    }

    // 2) enviar fora do lock; registrar sockets que falharem
    std::vector<int> to_remove;
    for (auto &sess : sessions_copy) {
        // Obter o nome (ou identificador) do remetente
        std::string sender_name;
        {
            std::lock_guard<std::mutex> lg(list_mutex_);
            auto it = sessions_.find(from_socket);
            if (it != sessions_.end()) {
                sender_name = it->second->getUsername(); // ou getClientName() / getId() conforme seu código
            } else {
                sender_name = "Cliente_" + std::to_string(from_socket);
            }
        }

        // Formatar mensagem com nome e quebra de linha
        std::string formatted_message = sender_name + ": " + message + "\n";

        // 1) copiar as sessões sob lock
        std::vector<std::shared_ptr<ClientSession>> sessions_copy;
        {
            std::lock_guard<std::mutex> lg(list_mutex_);
            sessions_copy.reserve(sessions_.size());
            for (const auto &p : sessions_) {
                if (p.first == from_socket) continue; // não reenviar ao remetente
                sessions_copy.push_back(p.second);
            }
        }

        // 2) enviar fora do lock
        std::vector<int> to_remove;
        for (auto &sess : sessions_copy) {
            if (!sess) continue;
            bool ok = sess->sendMessage(formatted_message);
            if (!ok) {
                to_remove.push_back(sess->getSocket());
            }
        }

        // 3) remover quem falhou
        if (!to_remove.empty()) {
            std::lock_guard<std::mutex> lg(list_mutex_);
            for (int fd : to_remove) {
                sessions_.erase(fd);
                TSLOG(INFO, "Removendo cliente desconectado (socket " + std::to_string(fd) + ")");
            }
        }

    }

    // 3) remover sessões que falharam (com lock)
    if (!to_remove.empty()) {
        std::lock_guard<std::mutex> lg(list_mutex_);
        for (int fd : to_remove) {
            auto it = sessions_.find(fd);
            if (it != sessions_.end()) {
                TSLOG(INFO, "Removendo cliente (socket " + std::to_string(fd) + ") após falha de envio.");
                sessions_.erase(it);
            }
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