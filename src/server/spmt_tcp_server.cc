#include "spmt_tcp_server.hpp"
#include "common.hpp"
#include <cstring>
#include <malloc.h>
#include <unistd.h>
void *handle_client(void *arg) {
  int clnt_fd = *(int *)arg;
  free(arg);
  char buf[BUF_SIZE]{0};
  while (true) {
    memset(buf, 0, BUF_SIZE);
    int read_len = read(clnt_fd, buf, BUF_SIZE);
    if (read_len <= 0) {
      break;
    }
    printf("recv data from client: %s\n", buf);
    // ehco 回显
    writelen(clnt_fd, buf, read_len);
  }
  close(clnt_fd);
  return nullptr;
}
