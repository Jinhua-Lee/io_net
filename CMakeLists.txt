cmake_minimum_required(VERSION 3.20)
project(io_net)

set(CMAKE_CXX_STANDARD 14)

# 服务端运行主函数
add_executable(server server/server_main.cpp common/common.cpp)

# 客户端运行主函数
add_executable(client client/client_main.cpp common/common.cpp)
