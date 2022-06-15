#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../common/common.h"

int select_client() {

    // 初始化IP
    struct sockaddr_in server_address = init_server_address();
    // 创建客户端的套接字FD
    int conn_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 连接到服务器
    int fail = connect(conn_fd, (struct sockaddr *) &server_address, sizeof(server_address));
    if (fail) {
        return fail;
    }

    char buf[BUFSIZ];
    do {
        fgets(buf, sizeof(buf), stdin);
        write(conn_fd, buf, strlen(buf));
        size_t n_bytes = read(conn_fd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, n_bytes);
        // 接收键盘输入，直到收到回车符结束
    } while (buf[0] != '\n');

    return 0;
}