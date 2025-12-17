#ifndef TSLOG_H
#define TSLOG_H

#include <string>
#include <fstream>
#include <mutex>
#include <iostream>
#include <chrono>

// Um enum simples para níveis de log
enum LogLevel { DEBUG, INFO, WARNING, ERROR };

// A classe Logger implementada como um Singleton para fácil acesso em todo o programa.
class ThreadSafeLogger {
private:
    std::ofstream log_file;
    std::mutex log_mutex;
    static ThreadSafeLogger* instance;

    // Construtor privado para garantir o padrão Singleton
    ThreadSafeLogger(const std::string& filename = "chat_server.log") {
        log_file.open(filename, std::ios_base::app); // Abre em modo append
    }

public:
    // Evita cópia e movimentação
    ThreadSafeLogger(const ThreadSafeLogger&) = delete;
    ThreadSafeLogger& operator=(const ThreadSafeLogger&) = delete;

    // Método de acesso ao Singleton
    static ThreadSafeLogger& getInstance() {
        if (instance == nullptr) {
            instance = new ThreadSafeLogger();
        }
        return *instance;
    }
    
    // Destrutor para fechar o arquivo
    ~ThreadSafeLogger() {
        if (log_file.is_open()) {
            log_file.close();
        }
    }

    // A função principal de logging
    void log(LogLevel level, const std::string& message);
};

// Macro de conveniência para uso mais limpo no código
#define TSLOG(level, message) \
    ThreadSafeLogger::getInstance().log(level, message)

#endif // TSLOG_H