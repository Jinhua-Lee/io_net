#include<cstdio>
#include<cstdlib>
#include<unistd.h>
#include<cstring>
#include<arpa/inet.h>
#include<cctype>

#define SERV_PORT 6666

int main() {
    int i, j, n, maxi;
    int maxfd, listen_fd, conn_fd, sock_fd;
    int n_ready, client[FD_SETSIZE - 1];  //FD_SETSIZE=1024,定义数组client来储存已连接描述符，最多1023个
    char buf[BUFSIZ], str;

    struct sockaddr_in cli_address, serv_address;
    socklen_t cli_address_len;
    fd_set all_set, read_set;  //定义监听描述符集合all_set和发生事件描述符集合read_set

    bzero(&serv_address, sizeof(serv_address));
    serv_address.sin_family = AF_INET;
    serv_address.sin_port = htons(SERV_PORT); //端口号，将无符号短整型转换为网络字节顺序
    serv_address.sin_addr.s_addr = htonl(INADDR_ANY);  //一个主机可能有多个网卡，所以是本机的任意IP地址

    listen_fd = socket(AF_INET, SOCK_STREAM, 0); //AF_INET表示使用32位IP地址，SOCK_STREAM表示使用TCP连接
    bind(listen_fd, (struct sockaddr *) &serv_address, sizeof(serv_address));  //将服务器套接字地址与套接字描述符联系起来
    listen(listen_fd, 1024); //设置可监听的连接数量为1024

    maxfd = listen_fd; //初始化最大文件描述符为监听描述符listen_fd

    //初始化client数组，将数组所有元素置为-1
    maxi = -1;  //数组client储存的文件描述符的个数，初始化为-1
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;

    //初始化select监听文件描述符的集合
    FD_ZERO(&all_set);     //初始化监听集合
    FD_SET(listen_fd, &all_set);  //将监听描述符listen_fd添加到集合中

    while (1) {
        read_set = all_set;
        n_ready = select(maxfd + 1, &read_set, NULL, NULL, NULL); //select只监听可读事件，且为永久阻塞直到有事件发生
        if (n_ready < 0)
            perror("select error");

        //判断listen_fd是否发生事件，若发生，则处理新客户端连接请求
        if (FD_ISSET(listen_fd, &read_set)) {
            cli_address_len = sizeof(cli_address);
            conn_fd = accept(listen_fd, (struct sockaddr *) &cli_address, &cli_address_len);//与请求客户端建立连接
            printf("received from % s at port % d\n",
                   inet_ntop(AF_INET, &cli_address.sin_addr.s_addr, &str, sizeof(str)),
                   ntohs(cli_address.sin_port));  //打印该客户端的IP地址和端口号

            //将conn_fd赋值给client数组中第一个为-1的元素位置
            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] < 0) {
                    client[i] = conn_fd;
                    break;
                }
            }

            //判断select监听的文件描述符的个数是否超过上限
            if (i == FD_SETSIZE - 1)   //减1的原因是要考虑监听描述符listen_fd也属于select监控
            {
                fputs("too many clients\n", stderr);
                exit(1);
            }

            FD_SET(conn_fd, &all_set);  //向监控的文件描述符集合all_set中添加新的描述符conn_fd
            if (conn_fd > maxfd)
                maxfd = conn_fd;   //更新最大文件描述符值

            //保证maxi永远是client数组中最后一个非-1的元素的位置
            if (i > maxi)
                maxi = i;

            //如果n_ready=1，即只有一个发生事件的描述符，在此条件下必为listen_fd，则返回循环位置，继续调用select监控；否则继续向下执行
            --n_ready;
            if (n_ready == 0) {
                continue;
            }
        }

        //找到client数组中发生事件的已连接描述符，并读取、处理数据
        for (i = 0; i <= maxi; i++) {
            sock_fd = client[i];
            if (sock_fd < 0)  //已连接描述符失效，重新开始循环
                continue;

            if (FD_ISSET(sock_fd, &read_set)) {
                n = read(sock_fd, buf, sizeof(buf));
                if (n == 0) //当客户端关闭连接，服务端也关闭连接
                {
                    close(sock_fd);
                    FD_CLR(sock_fd, &all_set);  //解除select对该已连接文件描述符的监控
                    client[i] = -1;
                } else if (n > 0) {
                    for (j = 0; j < n; j++)
                        buf[j] = toupper(buf[j]);
                    sleep(2);
                    write(sock_fd, buf, n);
                }

                --n_ready;
                if (n_ready == 0) {
                    break;  //跳出for循环，还在while中
                }
            }
        }
    }
    close(listen_fd);
    return 0;
}
