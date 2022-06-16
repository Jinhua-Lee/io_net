cmake_minimum_required(VERSION 3.19)
project(io_net)

set(CMAKE_CXX_STANDARD 14)

# 服务端运行主函数
add_executable(server server/server_main.cpp common/common.cpp)

# 客户端运行主函数
add_executable(client client/client_main.cpp common/common.cpp)

# 测试相关运行主函数
add_executable(test test/test_main.cpp)
find_package(Threads REQUIRED)
target_link_libraries(test Threads::Threads)