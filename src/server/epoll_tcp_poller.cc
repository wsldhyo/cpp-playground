#include "epoll_tcp_poller.hpp"
#include "epoll_tcp_channel.hpp"
#include <cstddef>
#include <sys/epoll.h>
#include <unistd.h>

int poller_init(Poller *poller) {
  poller->epfd = epoll_create1(0);
  return poller->epfd == -1 ? -1 : 0;
}

void poller_destroy(Poller *poller) {
  if (poller->epfd >= 0) {
    close(poller->epfd);
    poller->epfd = -1;
  }
}

int poller_add(Poller *poller, ETcpChannel *etch) {
  struct epoll_event ev = {0};
  ev.events = etch->events; // ETcpChannel 关注的事件加入epoll
  ev.data.ptr = etch;       // 事件就绪时，以找到关联的ETcpChannel
  return epoll_ctl(poller->epfd, EPOLL_CTL_ADD, etch->fd, &ev);
}

int poller_mod(Poller *poller, ETcpChannel *etch) {
  struct epoll_event ev{0};
  ev.events = etch->events;
  ev.data.ptr = etch;
  return epoll_ctl(poller->epfd, EPOLL_CTL_MOD, etch->fd, &ev);
}

int poller_del(Poller *poller, ETcpChannel *etch) {
  return epoll_ctl(poller->epfd, EPOLL_CTL_DEL, etch->fd, NULL);
}

// Poller核心接口：等待事件，并把实际事件写入 ETcpChannel
int poller_poll(Poller *poller, int timeout) {
  int n = epoll_wait(poller->epfd, poller->events, MAX_EVENTS, timeout);

  for (int i = 0; i < n; ++i) {
    ETcpChannel *etch = (ETcpChannel *)poller->events[i].data.ptr;
    etch->revents = poller->events[i].events;
  }

  return n;
}