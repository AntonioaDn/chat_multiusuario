# LPII - Trabalho Final: Servidor de Chat Multiusuário (Tema A)

**Objetivo:** Desenvolver um sistema concorrente em rede completo (cliente/servidor TCP) que demonstre domínio dos conceitos de threads, exclusão mútua, semáforos, variáveis de condição, monitores e sockets.

---

## Etapa 1: libtslog + Arquitetura (Tag: v1-logging)

Esta etapa focou na definição da arquitetura e na criação da biblioteca obrigatória de *logging* thread-safe, conforme a exigência do trabalho.

### 1. Detalhamento da libtslog

A **`libtslog`** foi implementada seguindo o padrão **Singleton** e utiliza **Exclusão Mútua** (`std::mutex`) em sua função `log()`.

* **Thread Safety:** O uso de um `std::lock_guard` garante que apenas uma *thread* por vez possa acessar e escrever no *stream* de arquivo do log. Isso previne *race conditions* e garante que a escrita de cada linha do log seja **atômica**, mantendo a integridade dos dados, mesmo sob concorrência intensa (conforme testado pelo `tslog_test`).

### 2. Arquitetura de Classes e Concorrência

A arquitetura define as classes que representam os conceitos centrais de concorrência exigidos pelo trabalho:

* **Threads:** `ChatServer` (thread principal de aceitação) e `ClientSession` (thread de trabalho por cliente).
* **Monitores:** `ClientManager` e `ThreadSafeQueue`.

#### Diagrama de Classes

```mermaid
classDiagram
    direction BT
    
    class ThreadSafeLogger {
        +getInstance(): ThreadSafeLogger*
        +log(level, message)
        -log_mutex: std::mutex
    }

    class ChatServer {
        +start()
        -startAcceptLoop()
        -server_socket_fd: int
    }

    class ClientManager {
        <<Monitor>>
        +addClient(socket_fd, username)
        +removeClient(socket_fd)
        +broadcastMessage(sender_fd, message)
        -clients: map<int, shared_ptr<ClientInfo>>
        -list_mutex: std::mutex
    }
    
    class ClientSession {
        +start()
        +sendMessage(message)
        -run(): void
        -client_socket_fd: int
        -worker_thread: std::thread
    }
    
    class ThreadSafeQueue {
        <<Monitor>>
        +push(T item)
        +wait_and_pop(): T
        -mutex: std::mutex
        -cond_var: std::condition_variable
    }
    
    ChatServer "1" --o "1" ClientManager: gerencia
    ChatServer "1" --* "N" ClientSession: cria thread por cliente
    ClientSession ..> ThreadSafeLogger: utiliza (TSLOG)
    ClientSession ..> ClientManager: notifica/broadcasting
    ChatServer ..> ThreadSafeQueue: utiliza (para controle interno/sincronização)
````

### 3\. Diagrama de Sequência (Fluxo Básico de Broadcast)

O diagrama ilustra o fluxo de dados e o uso de primitivas de sincronização (locks) durante o processo de conexão e retransmissão de mensagens (broadcast).

```mermaid
sequenceDiagram
    participant C as Cliente CLI
    participant S as ChatServer (Acceptor Thread)
    participant CM as ClientManager (Monitor)
    participant CS as ClientSession (Worker Thread)
    participant TSL as ThreadSafeLogger

    C->>S: 1. CONNECT (Solicita Conexão TCP) 
    S->>S: 2. accept() (bloqueia até a conexão)
    S->>CS: 3. Cria ClientSession (new std::thread) 
    S->>TSL: 4. log(INFO, "Nova conexão aceita")
    
    CS->>CM: 5. addClient(socket, username)
    CM->>CM: 6. LOCK list_mutex (Exclusão Mútua) 
    CM->>CM: 7. Adiciona cliente ao mapa
    CM->>CM: 8. UNLOCK list_mutex
    
    CS->>CS: 9. Loop: Espera por Mensagens (socket read)
    
    C->>CS: 10. Envia Mensagem M1
    CS->>CM: 11. broadcastMessage(sender_fd, M1)
    
    CM->>CM: 12. LOCK list_mutex
    CM->>CM: 13. Itera sobre clients_ (estrutura compartilhada)
    CM-->>CS: 14. sendMessage(M1) (para cada outra sessão) 
    CM->>CM: 15. UNLOCK list_mutex
    CS->>TSL: 16. log(DEBUG, "Broadcast enviado")
    
    C->>CS: 17. Disconnect / EOF
    CS->>CM: 18. removeClient(socket)
```

-----

**Status de Entrega da Etapa 1:**

  * **Código:** Headers principais (`.h`), `libtslog` e `tslog_test.cpp` concluídos.
  * **Build:** `CMakeLists.txt` funcional em Linux.
  * **Teste:** `tslog_test` executado com sucesso, provando a thread-safety do logger.