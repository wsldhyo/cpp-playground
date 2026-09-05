#ifndef EPOLL_TCP_EVENTLOOP_HPP
#define EPOLL_TCP_EVENTLOOP_HPP
#include "epoll_tcp_connection.hpp"
#include "epoll_tcp_poller.hpp"
typedef struct EventLoop {
  Poller poller; // 用于等待就绪Channel
  int quit;      // 是否结束
} EventLoop;

void event_loop_init(EventLoop* loop);
void event_loop_destroy(EventLoop* loop);
void event_loop_run(EventLoop *loop); 
void event_loop_quit(EventLoop *loop); // quit = 0
                                       //
#endif                                 // EPOLL_TCP_EVENTLOOP_HPP