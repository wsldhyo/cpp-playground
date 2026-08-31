#include "iouring_connection.hpp"

#include <cstdlib>
#include <cstring>
#include <cstdio>

#include <liburing.h>

#include "iouring_eventloop.hpp"
#include "iouring_request.hpp"

Connection *connection_create(struct EventLoop *loop, int fd) {
  Connection *conn = (Connection *)malloc(sizeof(Connection));

  if (!conn)
    return NULL;
  conn->loop = loop;
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
  Request *req = request_create(OP_READ);
  if (!req)
    return;
  // 存储请求的上下文信息
  req->loop = conn->loop;
  req->owner.conn = conn;
  req->fd = conn->fd;
  req->buf = conn->read_buf;
  req->len = sizeof(conn->read_buf);

  int ret = proactor_submit_read(&conn->loop->proactor, req, conn->fd,
                                 conn->read_buf, sizeof(conn->read_buf));

  if (ret < 0) {
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
  /*
   * result == 0：
   * 对端执行了 orderly shutdown，
   * 即 TCP EOF。
   */
  if (result == 0) {
    connection_close(conn);
    return;
  }
  /*
   * io_uring completion error
   * 通常是负 errno。
   */
  if (result < 0) {
    connection_close(conn);
    return;
  }

  /*
   * Echo：
   * 把收到的数据放进 write buffer。
   */
  if ((size_t)result > sizeof(conn->write_buf)) {
    connection_close(conn);
    return;
  }

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
  Request *req = request_create(OP_WRITE);

  if (!req)
    return;

  req->loop = conn->loop;
  req->owner.conn = conn;
  req->fd = conn->fd;
  req->buf = conn->write_buf + conn->write_offset;
  req->len = conn->write_len - conn->write_offset;
  int ret = proactor_submit_write(&conn->loop->proactor, req, conn->fd,
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

  /*
   * send 可能只发送了一部分。
   */
  conn->write_offset += (size_t)result;

  if (conn->write_offset < conn->write_len) {

    connection_start_write(conn);
    return;
  }

  /*
   * 整个 echo 数据已经发送完毕。
   */
  printf("send %ld bytes success\n", conn->write_len);
  conn->write_len = 0;
  conn->write_offset = 0;

  /*
   * 再次等待客户端数据。
   */
  connection_start_read(conn);
}