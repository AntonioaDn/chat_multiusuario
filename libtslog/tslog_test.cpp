// Arquivo: libtslog/tslog_test.cpp

#include "tslog.h"
#include <thread>
#include <vector>
#include <sstream>

void logger_thread(int id, int num_logs) {
    std::stringstream ss;
    for (int i = 0; i < num_logs; ++i) {
        ss.str("");
        ss << "Thread " << id << " logou a mensagem número " << i << ".";
        TSLOG(INFO, ss.str());
    }
}

int main() {
    const int NUM_THREADS = 5;
    const int LOGS_PER_THREAD = 10;
    std::vector<std::thread> workers;

    TSLOG(INFO, "Iniciando teste de log concorrente com " + std::to_string(NUM_THREADS) + " threads.");

    // Cria e inicia as threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        workers.emplace_back(logger_thread, i, LOGS_PER_THREAD); // Uso de std::thread 
    }

    // Espera todas as threads terminarem
    for (auto& t : workers) {
        if (t.joinable()) {
            t.join();
        }
    }

    TSLOG(INFO, "Teste de log concorrente finalizado. Verifique o arquivo 'chat_server.log'.");
    return 0;
}