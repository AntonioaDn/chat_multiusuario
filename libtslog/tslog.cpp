// Conteúdo Chave de libtslog/tslog.cpp
#include "tslog.h"
// ... (outros includes)

ThreadSafeLogger* ThreadSafeLogger::instance = nullptr;

void ThreadSafeLogger::log(LogLevel level, const std::string& message) {
    // 1. Bloqueia o mutex
    std::lock_guard<std::mutex> lock(log_mutex); // RAII para proteção contra exceções
    
    // 2. Formata a mensagem com timestamp e nível
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string time_str = std::ctime(&now);
    time_str.pop_back(); // Remove o '\n'

    std::string level_str;
    switch (level) {
        case DEBUG: level_str = "DEBUG"; break;
        case INFO: level_str = "INFO"; break;
        case WARNING: level_str = "WARN"; break;
        case ERROR: level_str = "ERROR"; break;
    }

    std::string final_message = "[" + time_str + "] [" + level_str + "] " + message;
    
    // 3. Escreve no arquivo e no console (opcional, mas útil)
    if (log_file.is_open()) {
        log_file << final_message << std::endl;
    }
    std::cout << final_message << std::endl;

    // 4. O mutex é liberado automaticamente ao sair do escopo do lock_guard
}