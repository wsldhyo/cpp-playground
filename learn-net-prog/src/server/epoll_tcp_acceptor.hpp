#ifndef EPOLL_TCP_ACCEPTOR
#define EPOLL_TCP_ACCEPTOR
#include "epoll_tcp_connection.hpp"
struct EventLoop;
typedef struct {
  int listen_fd;
  ETcpChannel channel;
  EventLoop *loop;
} Acceptor;

void acceptor_init(Acceptor *acceptor, EventLoop *loop, int listen_fd);

void acceptor_destroy(Acceptor *acceptor);

void acceptor_handle_read(ETcpChannel *etch);

void acceptor_handle_error(ETcpChannel *etch);

#endif // EPOLL_TCP_ACCEPTOR