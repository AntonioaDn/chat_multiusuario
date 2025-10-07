#ifndef MESSAGE_HISTORY_H
#define MESSAGE_HISTORY_H

#include <vector>
#include <string>
#include <mutex>
#include <sstream>

// Definir um limite razoável para o histórico
const size_t HISTORY_MAX_SIZE = 100;

class MessageHistory {
private:
    std::vector<std::string> history_messages_;
    mutable std::mutex history_mutex_; // Exclusão Mútua para proteger o vector

public:
    // Construtor
    MessageHistory() = default;

    // Adiciona uma mensagem ao histórico de forma thread-safe
    void addMessage(const std::string& sender, const std::string& message);

    // Retorna a lista completa (ou parte) do histórico de forma thread-safe
    std::vector<std::string> getHistory() const;

    // Método opcional para obter as últimas N mensagens
    std::vector<std::string> getLastN(size_t n) const;
};

#endif // MESSAGE_HISTORY_H