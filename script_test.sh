#!/bin/bash

# --- Configurações ---
SERVER_HOST="127.0.0.1"
SERVER_PORT=8080
NUM_CLIENTS=5 # Número de clientes para simular
MESSAGES_PER_CLIENT=10
SERVER_EXECUTABLE="./build/chat_server"
CLIENT_EXECUTABLE="./build/chat_client"

# --- Funções ---
function cleanup {
    echo "Encerrando o servidor e os clientes..."
    # Encerra o processo do servidor gentilmente (SIGTERM)
    if ps -p "$SERVER_PID" > /dev/null 2>&1; then
        kill -TERM "$SERVER_PID" 2>/dev/null || true
        # Aguarda um curto período pelo encerramento
        for i in {1..5}; do
            if ! ps -p "$SERVER_PID" > /dev/null 2>&1; then
                break
            fi
            sleep 1
        done
        # Se ainda estiver rodando, força o kill
        if ps -p "$SERVER_PID" > /dev/null 2>&1; then
            kill -KILL "$SERVER_PID" 2>/dev/null || true
        fi
    fi
    # Garante que todos os processos clientes (se ainda existirem) sejam encerrados
    for pid in "${CLIENT_PIDS[@]:-}"; do
        if [[ -n "$pid" ]] && ps -p "$pid" > /dev/null 2>&1; then
            kill -TERM "$pid" 2>/dev/null || true
        fi
    done
    echo "Limpeza concluída."
}

# --- Principal ---
echo "Compilando o projeto..."
# Garante que o projeto está compilado
cd build && make
cd ..

echo "Iniciando o servidor em segundo plano..."
$SERVER_EXECUTABLE $SERVER_PORT &
# Captura o ID do processo do servidor (PID)
SERVER_PID=$!

# Aguarda um pouco para o servidor iniciar completamente
sleep 2

# Registra a função de limpeza para ser executada na saída do script
trap cleanup EXIT


echo "Iniciando $NUM_CLIENTS clientes..."

# Inicia múltiplos clientes em segundo plano e guarda os PIDs para esperar somente por eles
CLIENT_PIDS=()
for i in $(seq 1 $NUM_CLIENTS); do
    (
        # Cria uma sequência de mensagens para enviar
        INPUT=""
        for j in $(seq 1 $MESSAGES_PER_CLIENT); do
            INPUT+="Mensagem $j do Cliente $i\\n"
        done
        
        # O 'echo -e' envia o nome e as mensagens para o stdin do cliente
        echo -e "Cliente$i\\n$INPUT" | $CLIENT_EXECUTABLE $SERVER_HOST $SERVER_PORT
    ) &
    CLIENT_PIDS+=("$!")
done

echo "Clientes iniciados. Aguardando a conclusão do envio de mensagens..."
# Espera explicitamente pelos clientes (não pelo servidor)
for pid in "${CLIENT_PIDS[@]}"; do
    if [[ -n "$pid" ]]; then
        wait "$pid" || true
    fi
done

echo "Teste concluído."
# A função cleanup será chamada automaticamente aqui

# Opcional: Mostra o final do log do servidor
echo "--- Final do Log do Servidor ---"
tail -n 20 chat_server.log