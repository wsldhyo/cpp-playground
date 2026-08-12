#include "common.hpp"
#include "cstring"
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
void* handle_client(void *arg) {
  int client_fd = *(int *)arg;
  free(arg);
  char buf[BUF_SIZE]{0};
  while (true) {
    memset(buf, 0, BUF_SIZE);
    int n = read(client_fd, buf, BUF_SIZE);
    if (n <= 0) {
      break;
    }
    std::cout << "recv data from client: " << buf << '\n';
    // ehco 回显
    write(client_fd, buf, n);
  }
  close(client_fd);
  return nullptr;
}
