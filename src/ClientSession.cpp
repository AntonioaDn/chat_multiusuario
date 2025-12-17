#include "ClientSession.h"
#include "ClientManager.h"
#include "../libtslog/tslog.h"

#include <sys/socket.h>   // send()
#include <unistd.h>       // close()
#include <cerrno>         // errno
#include <cstring>        // strerror()
#include <chrono>
#include <thread>
#include <string>

// se não existir MSG_NOSIGNAL, define depois dos includes
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif


#define BUFFER_SIZE 1024

// Construtor
ClientSession::ClientSession(int socket_fd, 
                             std::shared_ptr<ClientManager> manager,
                             std::shared_ptr<MessageHistory> history) 
    : client_socket_fd_(socket_fd), 
      manager_(manager),
      history_(history) 
{
    TSLOG(DEBUG, "Sessão criada para o socket " + std::to_string(client_socket_fd_));
    // Coloque aqui o restante do corpo do construtor
}

// Inicia a thread de trabalho, executando o método run().
void ClientSession::start() {
    worker_thread_ = std::thread(&ClientSession::run, this);
}

// O loop principal da thread de trabalho. 
void ClientSession::run() {
    char buffer[BUFFER_SIZE];
    int bytes_read;

    TSLOG(INFO, "Thread de sessão " + username_ + " iniciada.");

    while ((bytes_read = read(client_socket_fd_, buffer, BUFFER_SIZE - 1)) > 0) {
        
        buffer[bytes_read] = '\0';
        std::string message(buffer);
        
        // Limpeza básica: remove o '\n' ou '\r'
        while (!message.empty() && (message.back() == '\n' || message.back() == '\r')) {
            message.pop_back();
        }

        if (message.empty()) continue;

        TSLOG(DEBUG, "Mensagem recebida de " + username_ + ": " + message);

        // Retransmite a mensagem (Broadcast)
        manager_->broadcastMessage(client_socket_fd_, message);
    }
    
    // Se o loop terminou (desconexão ou erro)
    if (bytes_read == 0) {
        TSLOG(INFO, username_ + " (socket " + std::to_string(client_socket_fd_) + ") desconectou.");
    } else { 
        TSLOG(ERROR, "Erro de leitura no socket " + std::to_string(client_socket_fd_));
    }
    
    // Liberação de recursos:
    manager_->removeClient(client_socket_fd_); 
    close(client_socket_fd_); 
    
    TSLOG(INFO, "Thread de sessão " + username_ + " finalizada.");
}

// Envia toda a mensagem com tratamento de partial writes.
// Retorna true se todos os bytes foram enviados, false em erro/cliente fechado.
bool ClientSession::sendMessage(const std::string &msg) {
    const char *buf = msg.data();
    size_t total = msg.size();
    size_t sent = 0;
    // Usar MSG_NOSIGNAL quando disponível evita gerar SIGPIPE.
    int flags = 0;
#ifdef MSG_NOSIGNAL
    flags |= MSG_NOSIGNAL;
#endif

    while (sent < total) {
        ssize_t n = ::send(client_socket_fd_, buf + sent, total - sent, flags);
        if (n > 0) {
            sent += static_cast<size_t>(n);
            continue;
        }
        if (n == 0) {
            // socket fechado
            TSLOG(DEBUG, "send() retornou 0 para socket " + std::to_string(client_socket_fd_));
            return false;
        }
        // n < 0 -> erro
        if (errno == EINTR) continue; // re-tentar
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // se socket non-blocking: opcionalmente usar select/poll; aqui re-tentamos com pequeno sleep
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        // Erros como EPIPE e ECONNRESET indicam que o cliente desconectou
        if (errno == EPIPE || errno == ECONNRESET) {
            TSLOG(INFO, "Cliente desconectado (send failed) no socket " + std::to_string(client_socket_fd_) + ": " + std::strerror(errno));
            return false;
        }

        // Outros erros são inesperados e merecem aviso
        TSLOG(WARNING, "Erro ao enviar para socket " + std::to_string(client_socket_fd_) + ": " + std::strerror(errno));
        return false;
    }
    return true;
}


// Destrutor
ClientSession::~ClientSession() {
    if (worker_thread_.joinable()) {
        worker_thread_.detach(); // Uso de detach para evitar deadlocks no destrutor
    }
    TSLOG(DEBUG, "ClientSession destruída para o socket " + std::to_string(client_socket_fd_));
}