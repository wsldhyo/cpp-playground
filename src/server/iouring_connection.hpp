#ifndef IOURING_CONNECTION_HPP
#define IOURING_CONNECTION_HPP
#include "common.hpp"

// struct Proactor;
//
// typedef struct Connection {
//   struct Proactor *proactor;
//   int fd;
//   char read_buf[BUF_SIZE];
//   char write_buf[BUF_SIZE];
//   size_t write_len;
//   size_t write_offset;
//   int closed;
//   std::atomic_int ref_count; // 引用计数，用于生命周期管理
// } Connection;
//
// // 引用计数管理
// void connection_ref(Connection *conn);
// void connection_unref(Connection *conn); // 计数减为 0 时释放
//
// // 创建/销毁连接
// Connection *connection_create(struct Proactor *proactor, int fd);
// void connection_destroy(Connection *conn);
//
// // 提交异步读、写、关闭连接
// void submit_read(Connection *conn);
// void submit_write(Connection *conn);
// void connection_close(Connection *conn);

struct EventLoop;
typedef struct Connection {
  struct EventLoop *loop;
  int fd;
  char read_buf[BUF_SIZE];
  char write_buf[BUF_SIZE];
  size_t write_len;
  size_t write_offset;
  int read_pending;
  int write_pending;
  int closed;

} Connection;

Connection *connection_create(struct EventLoop *loop, int fd);

void connection_close(Connection *conn);

void connection_destroy(Connection *conn);

void connection_start_read(Connection *conn);

void connection_start_write(Connection *conn);

void connection_handle_read_completion(Connection *conn, int result);

void connection_handle_write_completion(Connection *conn, int result);
#endif // IOURING_CONNECTION_HPP