#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>

#define SERV_IP "127.0.0.1"
#define SERV_PORT 8081

sockaddr_in init_server_addr();

int main() {

    struct sockaddr_in server_address = init_server_addr();
    int conn_fd = socket(AF_INET, SOCK_STREAM, 0);

    int fail = connect(conn_fd, (struct sockaddr *) &server_address, sizeof(server_address));
    if (fail) {
        return fail;
    }

    char buf[BUFSIZ];
    while (true) {
        fgets(buf, sizeof(buf), stdin);
        write(conn_fd, buf, strlen(buf));
        size_t n_bytes = read(conn_fd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, n_bytes);
    }
}

sockaddr_in init_server_addr() {
    struct sockaddr_in server_address{};
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERV_PORT);
    // 将点十进制字节串转换为网络字节序
    inet_pton(AF_INET, SERV_IP, &server_address.sin_addr.s_addr);
    return server_address;
}
