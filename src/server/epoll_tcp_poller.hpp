#ifndef EPOLL_TCP_POLLER_HPP
#define EPOLL_TCP_POLLER_HPP
#include "common.hpp"
#include <sys/epoll.h>

struct ETcpChannel;
typedef struct Poller{
  int epfd;
  struct epoll_event events[MAX_EVENTS]; // 就绪事件数组
} Poller;

// 对epoll_ctl接口的封装，提供ETcpChannel与epoll实例的操作
int poller_init(Poller* poller);
void poller_destroy(Poller* poller);
int poller_add(Poller *poller, ETcpChannel *ch); // 添加 ETcpChannel到epoll实例
int poller_mod(Poller *poller, ETcpChannel *ch); // 修改 ETcpChannel
int poller_del(Poller *poller, ETcpChannel *ch); // 删除 ETcpChannel
int poller_poll(Poller *poller, int timeout);    // 等待 ETcpChannel关注的事件

#endif // EPOLL_TCP_POLLER_HPP