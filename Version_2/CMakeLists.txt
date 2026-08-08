cmake_minimum_required(VERSION 3.10)

project(ThreadPool VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Place executables in the binary folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/binary)

find_package(Threads REQUIRED)

# Server executable
add_executable(Server
    src/Server.cpp
)

target_link_libraries(Server PRIVATE Threads::Threads)

# Client executable
add_executable(Client
    src/Client.cpp
)

target_link_libraries(Client PRIVATE Threads::Threads)