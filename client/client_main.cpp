#include<cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>

#define SERV_IP "127.0.0.1"  //客户端、服务端都在一台主机上，所以直接用本机IP地址
#define SERV_PORT 6666

int main() {
    int cfd;
    struct sockaddr_in server_address{};
    char buf[BUFSIZ];
    int n;

    cfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, SERV_IP, &server_address.sin_addr.s_addr);  //将点十进制字节串转换为网络字节序

    connect(cfd, (struct sockaddr *) &server_address, sizeof(server_address));

    while (1) {
        fgets(buf, sizeof(buf), stdin);
        write(cfd, buf, strlen(buf));
        n = read(cfd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, n);
    }
    close(cfd);

    return 0;
}
