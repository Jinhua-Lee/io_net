#include "select_server.cpp"
#include "poll_server.cpp"
#include "epoll_server.cpp"

int main() {
    // debug operation must be in the project main.cpp, so we invoke select_server() here.

    select_server();
//    poll_server();
//    epoll_server();

    // as for client_main(), we can debug use telnet instead.
}
