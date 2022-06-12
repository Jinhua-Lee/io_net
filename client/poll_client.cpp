#include <sys/socket.h>
#include <netinet/in.h>
#include <cassert>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <poll.h>
#include <fcntl.h>
#include <iostream>

#include "../common/common.h"

#define BUFFER_SIZE 64

int poll_client() {
    struct sockaddr_in server_address = init_server_address();

    int sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock_fd >= 0);

    if (connect(sock_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        std::cout << "connection failed." << std::endl;
        close(sock_fd);
        return -1;
    }

    struct pollfd fds[2];
    //  添加标准输入
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    //  添加套接字描述符
    fds[1].fd = sock_fd;
    fds[1].events = POLLIN | POLLRDHUP;
    fds[1].revents = 0;

    char read_buf[BUFFER_SIZE];
    int pipe_fd[2];
    int ret = pipe(pipe_fd);
    assert(ret != -1);

    while (1) {
        ret = poll(fds, 2, -1);
        if (ret < 0) {
            printf("poll failure\n");
            break;
        }

        if (fds[1].revents & POLLRDHUP) {
            printf("server close the connection\n");
            break;
        } else if (fds[1].revents & POLLIN) {
            memset(read_buf, '\0', BUFFER_SIZE);
            recv(fds[1].fd, read_buf, BUFFER_SIZE - 1, 0);
            printf("%s\n", read_buf);
        }

        if (fds[0].revents & POLLIN) {
            ret = splice(0, NULL, pipe_fd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
            ret = splice(pipe_fd[0], NULL, sock_fd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
        }
    }

    close(sock_fd);
    return 0;
}