#include "ChatClient.h"
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

#define BUFFER_SIZE 1024

ChatClient::ChatClient() {
    TSLOG(INFO, "Cliente CLI inicializado.");
}

void ChatClient::connectToServer(const std::string& ip, int port) {
    if (connected_) {
        TSLOG(WARNING, "Já conectado. Desconecte antes de tentar novamente.");
        return;
    }

    // 1. Criação do Socket
    client_socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket_fd_ < 0) {
        TSLOG(ERROR, "Falha ao criar socket do cliente.");
        throw std::runtime_error("Falha ao criar socket.");
    }

    // 2. Configuração do Endereço do Servidor
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // Converte IP (string) para formato binário
    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
        TSLOG(ERROR, "Endereço IP inválido/não suportado.");
        close(client_socket_fd_);
        throw std::runtime_error("Endereço IP inválido.");
    }

    // 3. Conexão
    if (connect(client_socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        TSLOG(ERROR, "Falha na conexão com o servidor " + ip + ":" + std::to_string(port));
        close(client_socket_fd_);
        throw std::runtime_error("Falha na conexão.");
    }

    connected_ = true;
    TSLOG(INFO, "Conectado com sucesso ao servidor " + ip + ":" + std::to_string(port));

    // 4. Inicia a thread de recebimento
    receiver_thread_ = std::thread(&ChatClient::receiverLoop, this);
}

void ChatClient::sendMessage(const std::string& message) {
    if (!connected_) {
        std::cout << "ERRO: Não conectado. Use /connect primeiro." << std::endl;
        return;
    }
    
    // Adiciona uma quebra de linha para o servidor identificar o fim da mensagem
    std::string msg_with_newline = message + "\n";
    
    // Envia a mensagem
    if (send(client_socket_fd_, msg_with_newline.c_str(), msg_with_newline.length(), 0) < 0) {
        TSLOG(ERROR, "Erro ao enviar mensagem.");
    } else {
        TSLOG(DEBUG, "Mensagem enviada: " + message);
    }
}

// Thread dedicada à leitura de dados do servidor
void ChatClient::receiverLoop() {
    char buffer[BUFFER_SIZE];
    int bytes_read;
    
    TSLOG(INFO, "Thread de recebimento iniciada.");

    while (connected_ && (bytes_read = read(client_socket_fd_, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';
        std::string received_data(buffer);
        
        // **Saída Amigável no CLI:** Imprime a mensagem recebida sem o prefixo de log
        std::cout << received_data << std::flush;
    }

    // Se o loop terminou
    if (connected_) {
        TSLOG(WARNING, "Conexão perdida com o servidor.");
        connected_ = false;
    }
}

void ChatClient::disconnect() {
    if (connected_) {
        // Indica que não vamos enviar mais dados (fecha escrita), mas mantém a leitura
        connected_ = false;
        if (client_socket_fd_ >= 0) {
            // Fecha metade da conexão para sinalizar ao servidor que não vamos enviar mais
            ::shutdown(client_socket_fd_, SHUT_WR);
            // Dá um pequeno tempo para que a thread de recepção leia mensagens pendentes
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            ::close(client_socket_fd_);
            client_socket_fd_ = -1;
            TSLOG(INFO, "Conexão fechada.");
        }

        // Tenta juntar a thread de recebimento (se estiver rodando)
        if (receiver_thread_.joinable()) {
            receiver_thread_.join(); 
        }
    }
}

ChatClient::~ChatClient() {
    disconnect();
}