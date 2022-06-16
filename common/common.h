#include <cstring>
#include <arpa/inet.h>

#define SERV_PORT 8081

sockaddr_in init_server_address();

char * now_time();

void print_time(const char *time);