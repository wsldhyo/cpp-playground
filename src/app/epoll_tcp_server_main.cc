#include "epoll_tcp_acceptor.hpp"
#include "epoll_tcp_eventloop.hpp"
#include "common.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
int main(void) {
  EventLoop loop;
  event_loop_init(&loop);
  // 1. 创建 listen socket
  int listen_fd = create_socket(PF_INET, SOCK_STREAM, 0); 
  struct sockaddr_in addr{};
  bind_and_listen(listen_fd, BACKLOG, &addr, AF_INET, PORT);
  // 2. 设置非阻塞
  set_nonblocking(listen_fd);
  // 3. 创建 Acceptor
  Acceptor acceptor;
  acceptor_init(&acceptor, &loop, listen_fd);
  // 4. 进入 Reactor 事件循环
  event_loop_run(&loop);

  // 5.退出后清理
  acceptor_destroy(&acceptor);
  event_loop_destroy(&loop);
  return 0;
}