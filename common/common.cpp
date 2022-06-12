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