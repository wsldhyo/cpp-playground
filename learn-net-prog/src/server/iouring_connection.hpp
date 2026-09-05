#ifndef IOURING_CONNECTION_HPP
#define IOURING_CONNECTION_HPP
#include "common.hpp"

struct IOService;
typedef struct Connection {
  struct IOService *service;
  int fd;
  char read_buf[BUF_SIZE];
  char write_buf[BUF_SIZE];
  size_t write_len;
  size_t write_offset;
  int read_pending;  // 标记是否有读/写请求已经提交但未完成，防止重复提交
  int write_pending; 
  int closed;

} Connection;

Connection *connection_create(struct IOService *service, int fd);

void connection_close(Connection *conn);

void connection_destroy(Connection *conn);

void connection_start_read(Connection *conn);

void connection_start_write(Connection *conn);

void connection_handle_read_completion(Connection *conn, int result);

void connection_handle_write_completion(Connection *conn, int result);
#endif // IOURING_CONNECTION_HPP