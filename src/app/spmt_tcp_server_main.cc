#include "common.hpp"
#include "spmt_tcp_server.hpp"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
int main(int argc, char *argv[]) {
  // 创建socket
  int listen_sock_fd = socket(PF_INET, SOCK_STREAM, 0);
  if (listen_sock_fd < 0) {
    perror("socket()");
    return -1;
  }
  // 命名socket，绑定端口
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(listen_sock_fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind()");
    return -1;
  }
  // 创建监听队列，将socket转为LISTEN状态
  if (listen(listen_sock_fd, 128) < 0) {
    perror("listen()");
    return -1;
  }

  // 开始处理客户端连接请求
  std::cout << "server is ready to accept!\n";
  while (true) {
    // 接收客户端连接请求
    sockaddr_in new_clnt_addr{};
    socklen_t new_clnt_addr_len = sizeof(new_clnt_addr);
    int *new_clnt_fd = (int *)malloc(sizeof(int));
    *new_clnt_fd =
        accept(listen_sock_fd, (sockaddr *)&new_clnt_addr, &new_clnt_addr_len);
    if (*new_clnt_fd < 0) {
      perror("accept()");
      free(new_clnt_fd);
      continue; // 单个连接建立失败，不退出继续监听其他请求
    }
    std::cout << "new client\n";

    // 创建线程，处理客户端连接
    pthread_t tid;
    pthread_create(&tid, nullptr, handle_client, new_clnt_fd);
    pthread_detach(tid);
  }

  close(listen_sock_fd);
  return 0;
}