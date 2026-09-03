#include "iouring_connection.hpp"

#include <cstdlib>
#include <cstring>
#include <cstdio>

#include <liburing.h>

#include "iouring_io_service.hpp"
#include "iouring_request.hpp"

Connection *connection_create(struct IOService *service, int fd) {
  Connection *conn = (Connection *)malloc(sizeof(Connection));

  if (!conn)
    return NULL;
  conn->service = service;
  conn->fd = fd;
  conn->write_len = 0;
  conn->write_offset = 0;
  conn->read_pending = 0;
  conn->write_pending = 0;
  conn->closed = 0;
  return conn;
}

void connection_close(Connection *conn) {
  if (!conn || conn->closed)
    return;
  conn->closed = 1;
  close(conn->fd);
  conn->fd = -1;
}

void connection_destroy(Connection *conn) {
  if (!conn)
    return;
  connection_close(conn);
  free(conn);
}

void connection_start_read(Connection *conn) {
  if (conn->closed)
    return;
  if (conn->read_pending)
    return;
  Request *req = request_create(OP_RECV);
  if (!req)
    return;
  // 存储请求的上下文信息
  req->service = conn->service;
  req->owner.conn = conn;
  req->fd = conn->fd;
  req->buf = conn->read_buf;
  req->len = sizeof(conn->read_buf);

  int ret = io_backend_prep_recv(&conn->service->io_backend, req, conn->fd,
                                 conn->read_buf, sizeof(conn->read_buf));
  if (ret < 0) { // SQE 没能准备成功
    request_destroy(req);
    connection_close(conn);
    return;
  }
  conn->read_pending = 1;
}

void connection_handle_read_completion(Connection *conn, int result) {
  conn->read_pending = 0;
  if (conn->closed)
    return;
  if (result == 0) { // TCP EOF。对端关闭连接
    connection_close(conn);
    return;
  }
  // 负 errno，出错，直接关闭连接，无需像accepttor那样进一步判断
  if (result < 0) {
    connection_close(conn);
    return;
  }
  if ((size_t)result > sizeof(conn->write_buf)) {
    connection_close(conn);
    return;
  }
  // 正常读取数据，放入write buffer。并提交写请求
  memcpy(conn->write_buf, conn->read_buf, (size_t)result);
  printf("recv %d bytes\n", result);
  conn->write_len = (size_t)result;
  conn->write_offset = 0;
  connection_start_write(conn);
}

void connection_start_write(Connection *conn) {
  if (conn->closed)
    return;
  if (conn->write_pending)
    return;
  if (conn->write_offset >= conn->write_len)
    return;
  Request *req = request_create(OP_SEND);
  if (!req)
    return;
  // 保存异步上下文
  req->service = conn->service;
  req->owner.conn = conn;
  req->fd = conn->fd;
  req->buf = conn->write_buf + conn->write_offset;
  req->len = conn->write_len - conn->write_offset;
  int ret = io_backend_prep_send(&conn->service->io_backend, req, conn->fd,
                                 req->buf, req->len);
  printf("start send %ld bytes\n", req->len);
  if (ret < 0) {
    request_destroy(req);
    connection_close(conn);
    return;
  }
  conn->write_pending = 1;
}

void connection_handle_write_completion(Connection *conn, int result) {
  conn->write_pending = 0;
  if (conn->closed)
    return;
  if (result < 0) {
    connection_close(conn);
    return;
  }
  // send可能只发送一部分，重新提交写请求
  conn->write_offset += (size_t)result;
  if (conn->write_offset < conn->write_len) {
    connection_start_write(conn);
    return;
  }
  // 数据发送完毕。
  printf("send %ld bytes success\n", conn->write_len);
  conn->write_len = 0;
  conn->write_offset = 0;
  // 再次等待客户端数据
  connection_start_read(conn);
}