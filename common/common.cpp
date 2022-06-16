#include <ctime>
#include <cstdio>
#include "common.h"

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

char* now_time() {
    // static variable exists through all the lifetime.
    // without static, this variable will be cleaned up after function end, and return value will be null.
    // to solve this problem we can also use malloc(c) or new(c++) to allocate memory in heap.
    static char time_str[40];
    time_t t;
    tm *now;
    time(&t);
    now = localtime(&t);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", now);

    return &time_str[0];
}

void print_time(const char *time) {
    int index = 0;
    while (true) {
        if (time[index] == '\0') {
            break;
        }
        printf("%c", *(time + index++));
    }
}
