#include "iouring_eventloop.hpp"

#include <cstring>
#include <malloc.h>

#include "iouring_acceptor.hpp"
#include "iouring_connection.hpp"
#include "iouring_request.hpp"
int event_loop_init(EventLoop *loop) {
  int ret = proactor_init(&loop->proactor, 256);
  if (ret < 0)
    return ret;
  loop->running = 1;
  return 0;
}

void event_loop_run(EventLoop *loop) {
  while (loop->running) {
    Request *req = NULL;
    int result = 0;
    int ret = proactor_wait(&loop->proactor, &req, &result);
    if (ret == -EINTR)
      continue;
    if (ret < 0)
      break;
    event_loop_dispatch(loop, req, result);
  }
}

void event_loop_stop(EventLoop *loop) {
  loop->running = 0;
  // TOTO 唤醒正在等待CQE的loop
}

void event_loop_dispatch(EventLoop *loop, Request *req, int result) {
  (void)loop;

  switch (req->type) {
  case OP_ACCEPT:
    acceptor_handle_completion(req, result);
    break;
  case OP_READ:
    connection_handle_read_completion(req->owner.conn, result);
    request_destroy(req);
    break;
  case OP_WRITE:
    connection_handle_write_completion(req->owner.conn, result);
    request_destroy(req);
    break;
  }
}
void event_loop_destroy(EventLoop *loop) {
  event_loop_stop(loop);
  proactor_destroy(&loop->proactor);
}