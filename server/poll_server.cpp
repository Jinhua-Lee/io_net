#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <cstdlib>
#include <iostream>
#include "../common/common.h"

#include <sys/poll.h>

#define USER_LIMIT 5
#define BUFFER_SIZE 64
#define FD_LIMIT 65535

//  客户端的数据结构
typedef struct client_data {

    // 发送数据缓冲区
    char *write_buf;
    // 接收数据缓冲区
    char buf[BUFFER_SIZE];
} client_data;

void init_fd_struct_arr(int listen_fd, pollfd *fds, int length);

void handle_poll_in(client_data *users, pollfd *fds, int &user_counter, int &user_index);

void handle_poll_rdh_up(client_data *users, pollfd *fds, int &user_counter, int &user_index);

void handle_poll_err(const pollfd *fds, int user_index);

int set_non_blocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int poll_server() {

    int listen_fd;
    struct sockaddr_in address = init_server_address();

    client_data *users;

    // 创建服务器的监听套接字
    if ((listen_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("create socket error...\n");
        exit(-1);
    } else {
        printf("create socket success...\n");
    }

    // 绑定
    if (bind(listen_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("bind socket error...\n");
        exit(-1);
    } else {
        printf("bind socket success...\n");
    }

    // 监听
    if (listen(listen_fd, 5) < 0) {
        perror("listen error...\n");
    } else {
        printf("start listen...\n");
    }

    // 分配数据缓冲区
    if ((users = (client_data *) malloc(sizeof(client_data) * FD_LIMIT)) == nullptr) {
        perror("malloc client_data error...");
    } else {
        printf("malloc client_data success...");
    }

    // 初始化一个poll的结构体数组
    int fd_len = USER_LIMIT + 1;
    struct pollfd fds[USER_LIMIT + 1];
    init_fd_struct_arr(listen_fd, fds, fd_len);

    int user_counter = 0;
    while (true) {
        // 准备轮询的套接字文件描述符
        int ret_val = poll(fds,
                           user_counter + 1,
                // poll 永远等待。poll() 只有在一个描述符就绪时返回，或者在调用进程捕捉到信号时返回
                           -1);
        if (ret_val < 0) {
            std::cout << "poll failure." << std::endl;
            break;
        }

        // poll模型的本质就是轮询, 在poll()返回时，轮询所有的fd, 查找到有事件请求的fd
        for (int user_index = 0; user_index < user_counter + 1; ++user_index) {
            // 监听的是服务器套接字, 此时如果有数据可读，说明有客户端请求链接
            if ((fds[user_index].fd == listen_fd)
                && (fds[user_index].revents & POLLIN)) {
                struct sockaddr_in client_address{};
                socklen_t cli_addr_len = sizeof(client_address);

                //  开始接收客户端的链接
                int conn_fd = accept(listen_fd, (struct sockaddr *) &client_address, &cli_addr_len);
                if (conn_fd < 0) {
                    printf("errno is: %d\n", errno);
                    continue;
                }
                if (user_counter >= USER_LIMIT) {
                    const char *info = "too many users\n";
                    printf("%s", info);
                    send(conn_fd, info, strlen(info), 0);
                    close(conn_fd);
                    continue;
                }

                user_counter++;
                set_non_blocking(conn_fd);
                fds[user_counter].fd = conn_fd;
                fds[user_counter].events = POLLIN | POLLRDHUP | POLLERR;
                fds[user_counter].revents = 0;
                printf("comes a new user, now have %d users\n", user_counter);
            } else if (fds[user_index].revents & POLLERR) {
                // 数据出错
                handle_poll_err(fds, user_index);
            } else if (fds[user_index].revents & POLLRDHUP) {
                // 被挂起---断开
                handle_poll_rdh_up(users, fds, user_counter, user_index);
            } else if (fds[user_index].revents & POLLIN) {
                // 客户端套接字有数据可写
                handle_poll_in(users, fds, user_counter, user_index);
            } else if (fds[user_index].revents & POLLOUT) {
                // 服务器向外发送数据

                int conn_fd = fds[user_index].fd;
                if (!users[conn_fd].write_buf) {
                    continue;
                }
                send(conn_fd, users[conn_fd].write_buf, strlen(users[conn_fd].write_buf), 0);
                users[conn_fd].write_buf = nullptr;
                fds[user_index].events |= ~POLLOUT;
                fds[user_index].events |= POLLIN;
            }
        }
    }

    free(users);
    close(listen_fd);
    return 0;
}

void handle_poll_err(const pollfd *fds, int user_index) {
    printf("get an error from %d\n", fds[user_index].fd);
    char errors[100];
    memset(errors, '\0', 100);
    socklen_t length = sizeof(errors);
    if (getsockopt(fds[user_index].fd, SOL_SOCKET, SO_ERROR, &errors, &length) < 0) {
        printf("get socket option failed\n");
    }
}

void handle_poll_rdh_up(client_data *users, pollfd *fds, int &user_counter, int &user_index) {
    users[fds[user_index].fd] = users[fds[user_counter].fd];
    close(fds[user_index].fd);
    fds[user_index] = fds[user_counter];
    user_index--;
    user_counter--;
    printf("a client left\n");
}

void handle_poll_in(client_data *users, pollfd *fds, int &user_counter, int &user_index) {
    int conn_fd = fds[user_index].fd;
    memset(users[conn_fd].buf, '\0', BUFFER_SIZE);
    ssize_t ret = recv(conn_fd, users[conn_fd].buf, BUFFER_SIZE - 1, 0);
    printf("get %zd bytes of client data %s from %d\n", ret, users[conn_fd].buf, conn_fd);
    if (ret < 0) {
        if (errno != EAGAIN) {
            close(conn_fd);
            users[fds[user_index].fd] = users[fds[user_counter].fd];
            fds[user_index] = fds[user_counter];
            user_index--;
            user_counter--;
        }
    } else if (ret == 0) {
        printf("code should not come to here\n");
    } else {
        for (int j = 1; j <= user_counter; ++j) {
            if (fds[j].fd == conn_fd) {
                continue;
            }

            fds[j].events |= ~POLLIN;
            fds[j].events |= POLLOUT;
            users[fds[j].fd].write_buf = users[conn_fd].buf;
        }
    }
}

void init_fd_struct_arr(int listen_fd, pollfd *fds, int length) {

    // 初始化poll结构
    for (int i = 1; i <= length; ++i) {
        fds[i].fd = -1;
        fds[i].events = 0;
    }
    fds[0].fd = listen_fd;
    // POLLIN表示有数据可读, POLLERR表示出错
    fds[0].events = POLLIN | POLLERR;
    fds[0].revents = 0;
}
