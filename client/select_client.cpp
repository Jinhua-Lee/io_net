#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstdlib>
#include <ctime>
#include "../common/common.h"

void console_input(int conn_fd);

// random length and random content.
void random_input_to_server(int conn_fd);

void random_fill(char *buf);

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

    // replace by random input, in order to test concurrency input.
//    console_input(conn_fd);

    random_input_to_server(conn_fd);

    return 0;
}

void console_input(int conn_fd) {
    char buf[BUFSIZ];
    do {
        fgets(buf, sizeof(buf), stdin);
        write(conn_fd, buf, strlen(buf));
        size_t n_bytes = read(conn_fd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, n_bytes);
        // 接收键盘输入，直到收到回车符结束
    } while (buf[0] != '\n');
}

void random_input_to_server(int conn_fd) {
    char buf[BUFSIZ];
    random_fill(buf);
    write(conn_fd, buf, strlen(buf));
}

void random_fill(char *buf) {
    srand(time(nullptr));
    int len = rand() % 10;
    for (int i = 0; i < len; ++i) {
        srand(time(nullptr));
        // lower case letter.
        buf[i] = rand() % 26 + 97;
    }
}
