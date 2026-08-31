#include <unistd.h>
#include <cstdio>

#include "iouring_acceptor.hpp"
#include "iouring_eventloop.hpp"
#include "common.hpp"

int main(int argc, char *argv[]) {
  Acceptor acceptor{};
  EventLoop loop{};
  acceptor_init(&acceptor, &loop);
  if (event_loop_init(&loop) < 0) {
    acceptor_destroy(&acceptor);
    return 1;
  }
  printf("server start...listen port: %d\n", PORT);
  /*
   * 第一次主动发起 ACCEPT。
   */
  acceptor_start(&acceptor);
  /*
   * 之后 EventLoop 负责：
   *
   *   CQE
   *    ↓
   *   Request
   *    ↓
   *   Handler
   */
  event_loop_run(&loop);
  event_loop_destroy(&loop);
  acceptor_destroy(&acceptor);
  return 0;
}