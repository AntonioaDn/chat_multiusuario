#!/bin/bash
# Arquivo: test_multi_client.sh

# Porta do servidor (ajuste se necessário)
PORT=8080

# 1. Inicia o servidor em segundo plano e registra o PID
./build/chat_server $PORT &
SERVER_PID=$!
echo "Servidor iniciado com PID $SERVER_PID"

# Espera um pouco para o servidor iniciar
sleep 1

# 2. Inicia o cliente 1 (simulando um usuário)
echo "Cliente 1 conectado. Enviando 'Ola a todos!'"
(echo "USER CLIENTE1"; echo "Ola a todos!") | ./build/chat_client 127.0.0.1 $PORT &
CLIENT1_PID=$!

# 3. Inicia o cliente 2 (simulando outro usuário)
echo "Cliente 2 conectado. Enviando 'Broadcast funcionando?'"
(echo "USER CLIENTE2"; echo "Broadcast funcionando?") | ./build/chat_client 127.0.0.1 $PORT &
CLIENT2_PID=$!

# Espera os clientes terminarem as mensagens
sleep 2

# 4. Finaliza processos
kill $CLIENT1_PID $CLIENT2_PID $SERVER_PID
echo "Teste concluído. Verifique o chat_server.log."