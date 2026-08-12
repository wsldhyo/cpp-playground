#include "common.hpp"
#include "spmt_tcp_server.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

void accept_loop(int listen_sock_fd) {
  printf("server start...\n");
  while (true) {
    int clnt_fd = accept_client(listen_sock_fd);

    if (clnt_fd < 0)
      continue;

    printf("connected to new client\n");
    // 创建线程，处理客户端连接
    pthread_t tid;
    int* new_clnt_fd = (int*)malloc(sizeof(int));
    *new_clnt_fd = clnt_fd;
    pthread_create(&tid, nullptr, handle_client, new_clnt_fd);
    pthread_detach(tid);
  }
}

int main(int argc, char *argv[]) {
  int listen_sock_fd = create_socket(PF_INET, SOCK_STREAM, 0);
  sockaddr_in addr{};
  bind_and_listen(listen_sock_fd, BACKLOG, &addr, AF_INET, PORT);
  // 开始处理客户端连接请求
  std::cout << "server is ready to accept!\n";
  accept_loop(listen_sock_fd);
  close(listen_sock_fd);
  return 0;
}