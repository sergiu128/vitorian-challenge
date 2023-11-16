cmake_minimum_required(VERSION 3.13)

project(vitorian LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(
    -Wall
    -Wextra
    -Wno-c99-extensions
    -Wno-missing-field-initializers
    -Werror=format)

add_executable(client
    client.cpp
    tcp_client.cpp
    tcp_stream.cpp
    resolver.cpp
    proto_client.cpp
    net.cpp)

add_executable(test
    test.cpp
    tcp_client.cpp
    tcp_server.cpp
    tcp_stream.cpp
    resolver.cpp
    proto_client.cpp
    net.cpp)
