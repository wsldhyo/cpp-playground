#include <unistd.h>
#include <cstdio>

#include "iouring_acceptor.hpp"
#include "iouring_io_service.hpp"
#include "common.hpp"

int main(int argc, char *argv[]) {
  Acceptor acceptor{};
  IOService loop{};
  acceptor_init(&acceptor, &loop);
  if (io_service_init(&loop) < 0) {
    acceptor_destroy(&acceptor);
    return 1;
  }
  printf("server start...listen port: %d\n", PORT);
  // 第一次主动发起 ACCEPT。
  if(acceptor_start(&acceptor) < 0)
  {
    printf("start accept failed\n");
    return -1;
  }
  io_service_run(&loop);
  io_service_destroy(&loop);
  acceptor_destroy(&acceptor);
  return 0;
}