# CMakeLists.txt - CONTEÚDO CORRIGIDO E COMPLETO PARA ETAPA 2

cmake_minimum_required(VERSION 3.10)
project(LPII_ChatServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontra o módulo de threads (Correção da Etapa 1)
find_package(Threads REQUIRED) 

# --- libtslog (Etapa 1) ---
add_library(tslog STATIC libtslog/tslog.cpp)
target_include_directories(tslog PUBLIC libtslog)

# Executável de Teste da libtslog (Manter para referência)
add_executable(tslog_test libtslog/tslog_test.cpp)
target_link_libraries(tslog_test tslog Threads::Threads)

# --- ARQUIVOS CORE DA ETAPA 2 ---
# Define todos os arquivos de implementação do Servidor e Cliente em uma única biblioteca
add_library(chat_core STATIC
    src/ChatServer.cpp
    src/ClientManager.cpp
    src/ClientSession.cpp
    src/ChatClient.cpp 
    src/MessageHistory.cpp
)
# Inclui o diretório 'src' para que os headers se encontrem
target_include_directories(chat_core PUBLIC src)
# Linka o core com as bibliotecas necessárias (logging, threads e sockets)
target_link_libraries(chat_core tslog Threads::Threads)


# --- EXECUTÁVEIS DA ETAPA 2 ---

# 1. Executável do Servidor
add_executable(chat_server src/main_server.cpp)
target_link_libraries(chat_server chat_core tslog Threads::Threads)

# 2. Executável do Cliente CLI
add_executable(chat_client src/main_client.cpp)
target_link_libraries(chat_client chat_core tslog Threads::Threads)