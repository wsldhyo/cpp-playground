#include "epoll_tcp_channel.hpp"
#include "epoll_tcp_poller.hpp"
#include <cstdio>
#include <sys/epoll.h>
void channel_handle_event(ETcpChannel *etch) {
  // 处理异常事件
  if (etch->revents & (EPOLLERR | EPOLLHUP)) {
    if (etch->error_callback)
      etch->error_callback(etch);
    return;
  }
  // 处理可读事件
  if (etch->revents & EPOLLIN) {
    if (etch->read_callback)
      etch->read_callback(etch);
  }
  // 处理可写事件
  if (etch->revents & EPOLLOUT) {
    if (etch->write_callback)
      etch->write_callback(etch);
  }
  // 事件后处理
  // Connetction可能因为自己的写缓冲区满而停止Read，这里恢复Read
  if (etch->after_event_callback) {
    etch->after_event_callback(etch);
  }
}

void channel_enable_reading(Poller *poller, ETcpChannel *etch) {
  etch->events |= EPOLLIN;

  printf("[CHANNEL] enable EPOLLIN, fd=%d events=%u\n", etch->fd, etch->events);
  poller_mod(poller, etch);
}

void channel_disable_reading(Poller *poller, ETcpChannel *etch) {
  etch->events &= ~EPOLLIN;
  printf("[CHANNEL] disable EPOLLIN, fd=%d events=%u\n", etch->fd,
         etch->events);
  poller_mod(poller, etch);
}

void channel_enable_writing(Poller *poller, ETcpChannel *etch) {
  etch->events |= EPOLLOUT;

  printf("[CHANNEL] enable EPOLLOUT, fd=%d events=%u\n", etch->fd,
         etch->events);
  poller_mod(poller, etch);
}

void channel_disable_writing(Poller *poller, ETcpChannel *etch) {
  etch->events &= ~EPOLLOUT;

  printf("[CHANNEL] disable EPOLLOUT, fd=%d events=%u\n", etch->fd,
         etch->events);
  poller_mod(poller, etch);
}
