#include "MessageHistory.h"
#include <iostream>

void MessageHistory::addMessage(const std::string& sender, const std::string& message) {
    // 1. Bloqueio da exclusão mútua
    std::lock_guard<std::mutex> lock(history_mutex_); 

    std::stringstream ss;
    ss << "[" << sender << "]: " << message;
    
    // 2. Adiciona a mensagem formatada
    history_messages_.push_back(ss.str());

    // 3. Limpeza: Mantém apenas as últimas N mensagens
    if (history_messages_.size() > HISTORY_MAX_SIZE) {
        // Remove os elementos mais antigos
        history_messages_.erase(history_messages_.begin());
    }
    
    // O mutex é liberado automaticamente ao sair do escopo
}

std::vector<std::string> MessageHistory::getHistory() const {
    // Bloqueia para leitura
    std::lock_guard<std::mutex> lock(history_mutex_);
    
    // Retorna uma cópia do histórico (garantindo que o original não seja modificado
    // enquanto esta cópia é lida pela thread solicitante)
    return history_messages_;
}

std::vector<std::string> MessageHistory::getLastN(size_t n) const {
    std::lock_guard<std::mutex> lock(history_mutex_);
    
    size_t start_index = 0;
    if (history_messages_.size() > n) {
        start_index = history_messages_.size() - n;
    }
    
    // Retorna um subconjunto
    return std::vector<std::string>(
        history_messages_.begin() + start_index,
        history_messages_.end()
    );
}