#include "epoll_tcp_eventloop.hpp"
#include "epoll_tcp_channel.hpp"
#include <cstdio>
#include <errno.h>

void event_loop_init(EventLoop *loop) {
  loop->quit = 0;
  poller_init(&loop->poller);
}

void event_loop_destroy(EventLoop *loop) { poller_destroy(&loop->poller); }

void event_loop_run(EventLoop *loop) {
  printf("server start...\n");
  while (!loop->quit) {
    // 等待 I/O 事件
    int n = poller_poll(&loop->poller, -1);
    if (n < 0) {
      if (errno == EINTR)
        continue; // 被信号中断，重新等待
      break;      // epoll_wait 真正出错
    }
    // 分发所有就绪 Channel
    for (int i = 0; i < n; ++i) {
      ETcpChannel *etch = (ETcpChannel *)loop->poller.events[i].data.ptr;
      // 让 Channel 分发给具体 Handler
      channel_handle_event(etch);
    }
  }
}

// 该函数一般是在另一线程调用的，不会立即唤醒epoll_wait
// 实践中通常使用pipe，让epoll_wait因pipe可读而就绪唤醒
void event_loop_quit(EventLoop *loop) { loop->quit = 1; }
