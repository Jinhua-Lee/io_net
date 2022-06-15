#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<cstdio>
#include<cstring>
#include<unistd.h>
#include <fcntl.h>
#include<sys/epoll.h>
#include<vector>
#include<algorithm>

#include "../common/common.h"

// set block status of certain fd.
void set_fd_block(int fd, bool block);

typedef std::vector<struct epoll_event> EventList;

int epoll_server() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr = init_server_address();

    if (bind(listen_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("bind");
        return -2;
    }

    if (listen(listen_fd, 20) < 0) {
        perror("listen");
        return -3;
    }

    struct sockaddr_in conn_addr{};
    socklen_t len = sizeof(conn_addr);

    std::vector<int> clients;
    int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    //int epoll_fd = epoll_create(MAX_EVENTS);

    struct epoll_event event{};
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = listen_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) < 0) {
        perror("epoll_ctl");
        return -2;
    }
    EventList events(16);
    int count = 0;

    char buf[1024] = {0};
    while (true) {
        int n_ready = epoll_wait(epoll_fd,
                                 &*events.begin(),
                                 static_cast<int>(events.size()),
                                 -1);
        std::cout << "[epoll_wait] n_ready = " << n_ready << std::endl;
        if (n_ready == -1) {
            perror("epoll_wait");
            return -3;
        }
        // 肯定不会走到这里，因为上面没设置超时时间
        if (n_ready == 0) {
            continue;
        }
        // 对clients进行扩容
        if ((size_t) n_ready == events.size()) {
            events.resize(events.size() * 2);
        }
        // 开始轮询前n个有新事件的fd的结构体包装
        for (int ready_index = 0; ready_index < n_ready; ready_index++) {
            // 连接事件
            if (events[ready_index].data.fd == listen_fd) {
                int conn_fd = accept(listen_fd, (struct sockaddr *) &conn_addr, &len);
                if (conn_fd < 0) {
                    perror("accept");
                    return -4;
                }
                char ip_str[64] = {0};
                char *ip = inet_ntoa(conn_addr.sin_addr);
                strcpy(ip_str, ip);
                printf("[accept] client connect, conn_fd:%d,ip:%s, port:%d, count:%d\n",
                       conn_fd, ip_str, ntohs(conn_addr.sin_port),
                       ++count);

                clients.push_back(conn_fd);
                // 设为非阻塞
                set_fd_block(conn_fd, false);

                // add fd in events
                // 这样在epoll_wait返回时就可以直接用了
                event.data.fd = conn_fd;
                event.events = EPOLLIN | EPOLLET;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &event);
                std::cout << "[epoll_ctl_add] add conn_fd = " << conn_fd << " to the epoll ctl." << std::endl;

            } else if (events[ready_index].events & EPOLLIN) {
                // 写入事件

                int conn_fd = events[ready_index].data.fd;
                if (conn_fd < 0) {
                    continue;
                }
                ssize_t read_bytes = read(conn_fd, buf, sizeof(buf));
                if (read_bytes == -1) {
                    perror("read");
                    return -5;
                } else if (read_bytes == 0) {
                    printf("[read] client close remove conn_fd = %d, remaining count = %d\n", conn_fd, --count);
                    close(conn_fd);
                    event = events[ready_index];
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn_fd, &event);
                    std::cout << "[epoll_ctl_del] delete conn_fd = " << conn_fd << " from the epoll ctl." << std::endl;
                    clients.erase(
                            std::remove(clients.begin(), clients.end(), conn_fd),
                            clients.end()
                    );
                }
                write(conn_fd, buf, sizeof(buf));
                std::cout << "[write] write buf = " << buf << " to conn_fd = " << conn_fd << std::endl;
                memset(buf, 0, sizeof(buf));
            }
        }
    }
}

// set block status of certain fd.
void set_fd_block(int fd, bool block) {
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0)
        return;
    if (block) {
        flags &= ~O_NONBLOCK;
    } else {
        flags |= O_NONBLOCK;
    }

    if (fcntl(fd, F_SETFL, flags) < 0) {
        perror("fcntl set");
    }
}