#ifndef IOURING_ACCEPTOR_HPP
#define IOURING_ACCEPTOR_HPP
struct EventLoop;
struct Request;
typedef struct Acceptor {
  EventLoop *loop;
  int listen_fd;
} Acceptor;

void acceptor_init(Acceptor *acceptor, EventLoop *loop);
void acceptor_destroy(Acceptor *acceptor);
void acceptor_start(Acceptor *acceptor);
void acceptor_handle_completion(Request *req, int result);

#endif // IOURING_ACCEPTOR_HPP