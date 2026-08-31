// event_loop.h
#ifndef IOURING_EVENTLOOP_HPP
#define IOURING_EVENTLOOP_HPP
#include <liburing.h>
#include "iouring_proactor.hpp"
// struct Request;
// typedef struct EventLoop {
//   struct io_uring ring;
//   int listen_fd;
//   int running;
// } EventLoop;
// 
// int event_loop_init(EventLoop *loop, int listen_fd);
// void event_loop_run(EventLoop *loop);
// void submit_accept(EventLoop *loop);
// 
// void handle_read(Request *req, int result);
// void handle_write(Request *req, int result);
// void handle_accept(Request *req, int result);
// void handle_completion(Request *req, int result);

struct Request;
typedef struct EventLoop {
    Proactor proactor;
    int running;
} EventLoop;
int event_loop_init(EventLoop *loop);

void event_loop_run(EventLoop *loop);

void event_loop_stop(EventLoop *loop);

void event_loop_dispatch(EventLoop *loop, struct Request *req, int result);

void event_loop_destroy(EventLoop *loop);

#endif // IOURING_EVENTLOOP_HPP