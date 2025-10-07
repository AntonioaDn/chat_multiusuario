#include "ChatServer.h"
#include "ClientSession.h"
#include "MessageHistory.h"
#include <unistd.h>      // close()
#include <sys/socket.h>  // socket, bind, listen, accept
#include <netinet/in.h>  // sockaddr_in
#include <arpa/inet.h>   // inet_ntoa
#include <cstring>       // memset
#include <stdexcept>
#include <signal.h>

// Inicializa o ClientManager e a porta
// Em chat_multiusuario/src/ClientSession.cpp (Linha 22, onde o erro ocorre)
ChatServer::ChatServer(int port) : port_(port) {
    // Inicializa o ClientManager e o novo Monitor MessageHistory
    client_manager_ = std::make_shared<ClientManager>();
    message_history_ = std::make_shared<MessageHistory>();
    TSLOG(INFO, "Servidor inicializado na porta " + std::to_string(port) + ".");
    // Ignorar SIGPIPE globalmente: evita que writes para sockets fechados derrubem o processo
    signal(SIGPIPE, SIG_IGN);
}

// Inicia a thread principal de aceitação
void ChatServer::start() {
    // 1. Criação do Socket
    server_socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd_ < 0) {
        TSLOG(ERROR, "Falha ao criar socket.");
        throw std::runtime_error("Falha ao criar socket.");
    }
    
    // Configuração para reutilizar a porta rapidamente (SO_REUSEADDR)
    int opt = 1;
    if (setsockopt(server_socket_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        TSLOG(WARNING, "setsockopt falhou.");
    }

    // 2. Configuração de Endereço (IP e Porta)
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Aceita conexões em qualquer IP
    server_addr.sin_port = htons(port_);

    // 3. Bind
    if (bind(server_socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        TSLOG(ERROR, "Falha ao dar bind na porta.");
        close(server_socket_fd_);
        throw std::runtime_error("Falha ao dar bind na porta.");
    }

    // 4. Listen
    // O backlog (número 5) é a fila de conexões pendentes (requisito: Fila de conexões pendentes com limite configurável)
    if (listen(server_socket_fd_, 5) < 0) { 
        TSLOG(ERROR, "Falha ao iniciar listen.");
        close(server_socket_fd_);
        throw std::runtime_error("Falha ao iniciar listen.");
    }

    TSLOG(INFO, "Servidor TCP escutando em 0.0.0.0:" + std::to_string(port_));
    
    // Lança a thread principal de aceitação (requisito: threads)
    acceptor_thread_ = std::thread(&ChatServer::startAcceptLoop, this);
    
    // Mantém a thread principal da aplicação viva, esperando a thread de aceitação
    if (acceptor_thread_.joinable()) {
        acceptor_thread_.join(); 
    }
}

// Loop principal que aceita e despacha clientes para novas threads
void ChatServer::startAcceptLoop() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (true) {
        // 5. Accept
        int client_socket = accept(server_socket_fd_, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            // Em um sistema real, você checaria errno. Aqui, apenas logamos e continuamos
            TSLOG(ERROR, "Erro ao aceitar conexão.");
            continue;
        }

        std::string client_ip = inet_ntoa(client_addr.sin_addr);
        TSLOG(INFO, "Nova conexão aceita de: " + client_ip + " no socket: " + std::to_string(client_socket));

        // 6. Cria e Inicia a Thread de Sessão (requisito: Cada cliente atendido por thread)
        try {
            auto session = std::make_shared<ClientSession>(
                client_socket, 
                client_manager_,
                message_history_
            );
            
            session->start();
            // A ClientManager gerencia a lista de sessões/sockets (protegida por mutex)
            client_manager_->addClient(session); 
        } catch (const std::exception& e) {
            TSLOG(ERROR, "Falha ao iniciar thread da sessão: " + std::string(e.what()));
            close(client_socket);
        }
    }
}

ChatServer::~ChatServer() {
    if (server_socket_fd_ >= 0) {
        close(server_socket_fd_);
    }
    if (acceptor_thread_.joinable()) {
        // Nota: Em produção, você faria um 'detach' ou usaria um flag para encerrar o loop.
        // Aqui, forçaremos o join para a demo da Etapa 2.
        acceptor_thread_.join(); 
    }
}