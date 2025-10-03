#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <memory>

template <typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cond_var_;

public:
    // Tenta adicionar um item na fila
    void push(T item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(item));
        cond_var_.notify_one(); // Sinaliza que um novo item foi adicionado
    }

    // Espera até que um item esteja disponível e o remove (bloqueante)
    T wait_and_pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Espera até que a fila não esteja vazia
        cond_var_.wait(lock, [this] { return !queue_.empty(); });

        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }
    
    // Tenta remover um item, retorna false se a fila estiver vazia (não bloqueante)
    bool try_pop(T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        item = std::move(queue_.front());
        queue_.pop();
        return true;
    }
    
    // Retorna o tamanho da fila (thread-safe)
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
};

#endif // THREAD_SAFE_QUEUE_H