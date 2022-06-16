#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include <iostream>
#include "../common/common.h"

// concurrency visit server, this is client num.
#define CLI_NUM 10000

pthread_mutex_t mut;

void console_input(int conn_fd);

// we define random length and random content.
void random_input_to_server(int conn_fd);

void random_fill(char *buf);

void *select_client(void *args);

void print_conn(const void *args);
void print_send(const void *args);

void select_clients_concurrency() {

    // create thread in loop
    for (int i = 0; i < CLI_NUM; ++i) {
        pthread_t p_th;
        pthread_create(&p_th, nullptr, select_client, &i);
    }

    getchar();
}

void *select_client(void *args) {

    // 初始化IP
    struct sockaddr_in server_address = init_server_address();
    // 创建客户端的套接字FD
    int conn_fd = socket(AF_INET, SOCK_STREAM, 0);

//    pthread_mutex_lock(&mut);

    // 连接到服务器
    int fail = connect(conn_fd, (struct sockaddr *) &server_address, sizeof(server_address));
    print_conn(args);
    if (fail) {
        return nullptr;
    }

    // replaced by random input, in order to test concurrency visit.
//    console_input(conn_fd);
    random_input_to_server(conn_fd);
    print_send(args);

//    pthread_mutex_unlock(&mut);
    return nullptr;
}

void print_conn(const void *args) {
    std::cout << "[client connect] thread = " << *(int *)args << ", time = ";
    print_time(now_time());
    std::cout << std::endl;
}

void print_send(const void *args) {
    std::cout << "[client send] thread = " << *(int *)args << ", time = ";
    print_time(now_time());
    std::cout << std::endl;
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
