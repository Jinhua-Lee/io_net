#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <cctype>
#include <iostream>

#define SERV_PORT 8081

sockaddr_in init_server_address();

fd_set &read_and_handle(int *client, fd_set &all_set, fd_set &read_set, int max_valid_index, int n_ready);

int main() {
    // AF_INET表示使用32位IP地址，SOCK_STREAM表示使用TCP连接
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_address = init_server_address();
    // 将 服务器套接字地址 与 套接字描述符 联系起来
    int fail = bind(listen_fd, (struct sockaddr *) &serv_address, sizeof(serv_address));
    if (fail) {
        std::cout << "bind port failure." << std::endl;
        return -1;
    }
    // 设置可监听的连接数量为1024
    listen(listen_fd, FD_SETSIZE);

    fd_set all_set, read_set;
    // 初始化select监听文件描述符的集合
    FD_ZERO(&all_set);
    FD_SET(listen_fd, &all_set);

    // 定义cli数组来储存已连接描述符，最多1023个
    int cli_fd_arr[FD_SETSIZE - 1];
    for (int cli_index = 0; cli_index < FD_SETSIZE; cli_index++) {
        cli_fd_arr[cli_index] = -1;
    }

    // 初始化最大文件描述符为监听描述符listen_fd
    int max_fd = listen_fd;
    int max_valid_index = -1;

    while (true) {
        read_set = all_set;
        // select只监听可读事件，且为永久阻塞直到有事件发生
        int n_ready = select(max_fd + 1, &read_set, nullptr, nullptr, nullptr);
        if (n_ready < 0) {
            perror("select error");
        }

        // 判断listen_fd是否发生事件，若发生，则处理新客户端连接请求
        if (FD_ISSET(listen_fd, &read_set)) {

            struct sockaddr_in cli_address{};
            socklen_t cli_address_len = sizeof(cli_address);
            // 与请求客户端建立连接
            int conn_fd = accept(listen_fd, (struct sockaddr *) &cli_address, &cli_address_len);
            char str;
            printf("received from %s at port % d\n",
                   inet_ntop(AF_INET, &cli_address.sin_addr, &str, sizeof(str)),
                   ntohs(cli_address.sin_port));

            int cli_index;
            // 将conn_fd赋值给client数组中第一个为-1的元素位置
            for (cli_index = 0; cli_index < FD_SETSIZE; cli_index++) {
                if (cli_fd_arr[cli_index] < 0) {
                    cli_fd_arr[cli_index] = conn_fd;
                    break;
                }
            }

            // 判断select监听的文件描述符的个数是否超过上限
            // 减1的原因是要考虑监听描述符listen_fd也属于select监控
            if (cli_index == FD_SETSIZE - 1) {
                fputs("too many clients\n", stderr);
                exit(1);
            }

            // 向监控的文件描述符集合all_set中添加新的描述符conn_fd
            FD_SET(conn_fd, &all_set);
            if (conn_fd > max_fd) {
                // 更新最大文件描述符值
                max_fd = conn_fd;
            }
            // 保证maxi永远是client数组中最后一个非-1的元素的位置
            if (cli_index > max_valid_index) {
                max_valid_index = cli_index;
            }

            // 如果n_ready=1，即只有一个发生事件的描述符，在此条件下必为listen_fd，则返回循环位置，继续调用select监控；
            // 否则继续向下执行
            --n_ready;
            if (n_ready == 0) {
                continue;
            }
        }

        all_set = read_and_handle(cli_fd_arr, all_set, read_set, max_valid_index, n_ready);
    }
}

fd_set &read_and_handle(int *client, fd_set &all_set, fd_set &read_set, int max_valid_index, int n_ready) {
    // 找到client数组中发生事件的已连接描述符，并读取、处理数据
    for (int cli_index = 0; cli_index <= max_valid_index; cli_index++) {
        char buf[BUFSIZ];
        int sock_fd = client[cli_index];
        // 已连接描述符失效，重新开始循环
        if (sock_fd < 0) {
            continue;
        }

        if (FD_ISSET(sock_fd, &read_set)) {
            size_t read_bytes = read(sock_fd, buf, sizeof(buf));
            // 当客户端关闭连接，服务端也关闭连接
            if (read_bytes == 0) {
                close(sock_fd);
                // 解除select对该已连接文件描述符的监控
                FD_CLR(sock_fd, &all_set);
                client[cli_index] = -1;
            } else if (read_bytes > 0) {
                for (int j = 0; j < read_bytes; j++) {
                    buf[j] = (char)toupper(buf[j]);
                }
                sleep(2);
                write(sock_fd, buf, read_bytes);
            }

            --n_ready;
            // 跳出for循环，还在while中
            if (n_ready == 0) {
                break;
            }
        }
    }
    return all_set;
}

sockaddr_in init_server_address() {
    struct sockaddr_in serv_address{};
    bzero(&serv_address, sizeof(serv_address));
    serv_address.sin_family = AF_INET;
    // 端口号，将无符号短整型转换为网络字节顺序
    serv_address.sin_port = htons(SERV_PORT);
    // 一个主机可能有多个网卡，所以是本机的任意IP地址
    serv_address.sin_addr.s_addr = htonl(INADDR_ANY);
    return serv_address;
}
