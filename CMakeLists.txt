cmake_minimum_required(VERSION 3.20)
project(io_net)

set(CMAKE_CXX_STANDARD 14)

add_executable(io_net main.cpp)
#add_executable(server server/select_main.cpp)
add_executable(client client/client_main.cpp)
