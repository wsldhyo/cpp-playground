#include "epoll_tcp_acceptor.hpp"
#include "epoll_tcp_eventloop.hpp"
#include <cerrno>
#include <cstdio>
#include <stddef.h>
#include <sys/epoll.h>
#include <unistd.h>
void acceptor_init(Acceptor *acceptor, EventLoop *loop, int listen_fd) {
  acceptor->listen_fd = listen_fd;
  acceptor->loop = loop;
  ETcpChannel *etch = &acceptor->channel;
  etch->fd = listen_fd;
  etch->events = EPOLLIN;
  etch->revents = 0;

  etch->read_callback = acceptor_handle_read;
  etch->write_callback = NULL;
  etch->error_callback = acceptor_handle_error;
  etch->after_event_callback = NULL;
  etch->owner = acceptor;

  // 将 listen fd 注册到 epoll
  poller_add(&loop->poller, etch);
}

void acceptor_destroy(Acceptor *acceptor) {
  if (acceptor->listen_fd >= 0) {
    poller_del(&acceptor->loop->poller, &acceptor->channel);

    close(acceptor->listen_fd);
    acceptor->listen_fd = -1;
  }
}

void acceptor_handle_read(ETcpChannel *etch) {
  Acceptor *acceptor = (Acceptor *)etch->owner;
  while (1) {
    int client_fd = accept(acceptor->listen_fd, NULL, NULL);
    if (client_fd < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
        break;
      break;
    }
    printf("new client connect\n");
    set_nonblocking(client_fd);
    // 创建 ETcpConnection
    connection_create(acceptor->loop, client_fd);
  }
}

void acceptor_handle_error(ETcpChannel *etch) {
  Acceptor *acceptor = (Acceptor *)etch->owner;

  int error = 0;
  socklen_t len = sizeof(error);

  // 获取监听 socket 的具体错误
  if (getsockopt(acceptor->listen_fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
    perror("getsockopt");
    return;
  }

  if (error != 0) {
    fprintf(stderr, "listen socket error: %s\n", strerror(error));

    // 监听 socket 出错通常无法继续 accept
    event_loop_quit(acceptor->loop);
  }
}