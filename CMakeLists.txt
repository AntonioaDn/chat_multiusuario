# CMakeLists.txt - CONTEÚDO CORRIGIDO

cmake_minimum_required(VERSION 3.10)
project(LPII_ChatServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. ENCONTRA O MÓDULO DE THREADS
# Isso garante que a funcionalidade de thread C++ seja encontrada e linkada corretamente
find_package(Threads REQUIRED) # <--- Linha nova e essencial

# 2. Biblioteca libtslog
add_library(tslog STATIC libtslog/tslog.cpp)
target_include_directories(tslog PUBLIC libtslog)

# 3. Executável de Teste da libtslog
add_executable(tslog_test libtslog/tslog_test.cpp)

# Linka com a libtslog e, crucialmente, com Threads::Threads
target_link_libraries(tslog_test tslog Threads::Threads) # <--- Linha corrigida