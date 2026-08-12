#include "common.hpp"
#include "mpst_tcp_server.hpp"
#include <csignal>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>

void accept_loop(int listen_sock_fd)
{
    printf("server start...\n");
    while (true) {
        int clnt_fd = accept_client(listen_sock_fd);

        if (clnt_fd < 0)
            continue;

        printf("connected to new client\n");
        handle_client_process(listen_sock_fd, clnt_fd);
    }
}

int main(int argc, char *argv[]) {
  signal(SIGCHLD, reap_child_process);
  // 创建套接字，并开始监听
  int listen_sock_fd = create_socket(PF_INET, SOCK_STREAM, 0);
  sockaddr_in addr{};
  bind_and_listen(listen_sock_fd, BACKLOG, &addr, AF_INET, PORT);
  accept_loop(listen_sock_fd);
  return 0;
}