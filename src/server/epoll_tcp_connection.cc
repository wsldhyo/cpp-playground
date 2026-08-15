#include "epoll_tcp_connection.hpp"
#include "epoll_tcp_channel.hpp"
#include "epoll_tcp_eventloop.hpp"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <malloc.h>
#include <sys/epoll.h>
#include <unistd.h>
ETcpConnection *connection_create(EventLoop *loop, int fd) {
  ETcpConnection *conn = (ETcpConnection *)malloc(sizeof(*conn));
  conn->fd = fd;
  conn->channel.fd = fd;
  conn->channel.owner = conn;
  conn->channel.events = EPOLLIN | EPOLLET;
  conn->loop = loop;
  conn->write_end = 0;
  conn->write_start = 0;
  conn->read_len = 0;
  conn->read_paused = 0;
  // 注册事件回调
  conn->channel.read_callback = connection_handle_read;
  conn->channel.write_callback = connection_handle_write;
  conn->channel.error_callback = connection_handle_error;
  conn->channel.after_event_callback = connection_after_event;
  poller_add(&loop->poller, &conn->channel);
  return conn;
}

void connection_handle_read(ETcpChannel *etch) {
  ETcpConnection *conn = (ETcpConnection *)etch->owner;

  while (1) {
    // 1. 背压检查（应用层缓冲区满）
    size_t pending = conn->write_end - conn->write_start;
    size_t free_space = BUF_SIZE - pending;
    if (free_space == 0) {
      conn->read_paused = 1;
      printf("[READ] output buffer full, pause reading\n");
      return; // 暂停读，等待 EPOLLOUT 恢复
    }

    // 2. 从内核 Socket 读取数据
    size_t read_size = (free_space < sizeof(conn->read_buf))
                           ? free_space
                           : sizeof(conn->read_buf);
    ssize_t n = read(conn->fd, conn->read_buf, read_size);

    if (n > 0) {
      // 3. 尝试发送（入队 + 条件刷出）
      int ret = connection_send(conn, conn->read_buf, n);

      if (ret == -1) {
        // 应用层缓冲区满（虽然刚检查过，但可能在并发？单线程不会，但保留防御）
        conn->read_paused = 1;
        return;
      } else if (ret == -2) {
        // 致命写错误（例如对端发 RST），连接已不可用，关闭并立即返回
        printf("[READ] fatal write error, closing\n");
        connection_close(conn);
        return; // 【关键】绝不 continue，也不访问 conn
      }
      // ret == 0，数据成功入队/发出，继续读取 Socket 中的数据（ET 模式特性）
      continue;

    } else if (n == 0) {
      // 对端正常关闭连接
      printf("[READ] client closed\n");
      connection_close(conn);
      return;

    } else { // n < 0
      if (errno == EINTR) {
        continue; // 被信号中断，重试读
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // 内核接收缓冲区已空，ET 模式下正常退出
        return;
      } else {
        // 真正的读错误
        perror("[READ] read error");
        connection_close(conn);
        return;
      }
    }
  }
}

void connection_handle_write(ETcpChannel *etch) {
  ETcpConnection *conn = (ETcpConnection *)etch->owner;
  // 如果刷出遇到致命错误，关闭连接
  if (connection_flush(conn) < 0) {
    connection_close(conn);
  }
}

void connection_handle_error(ETcpChannel *etch) {
  ETcpConnection *conn = (ETcpConnection *)etch->owner;
  int error = 0;
  socklen_t len = sizeof(error);
  // 获取具体错误
  if (getsockopt(conn->fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
    perror("getsockopt");
    connection_close(conn);
    return;
  }
  // 判断错误
  if (error != 0) {
    // 记录 / 上报错误
    fprintf(stderr, "socket error: %s\n", strerror(error));
    // TCP 连接的大多数 socket 错误无法继续使用
    connection_close(conn);
    return;
  }
}

void connection_after_event(ETcpChannel *etch) {
  ETcpConnection *conn = (ETcpConnection *)etch->owner;
  if (conn == NULL)
    return; // 连接可能已被关闭
  // 只有写事件发生才可能释放输出缓冲区空间
  if (!(etch->revents & EPOLLOUT)) {
    return;
  }
  if (!conn->read_paused) {
    return;
  }
  // 先前被暂停读了，恢复读避免数据长时间滞留
  conn->read_paused = 0;
  connection_handle_read(etch);
}
void connection_compact_write_buffer(ETcpConnection *conn) {
  if (conn->write_start == 0)
    return;
  if (conn->write_start == conn->write_end) {
    conn->write_end = 0;
    conn->write_start = 0;
    return;
  }
  size_t pending_len = conn->write_end - conn->write_start;
  memmove(conn->write_buf, conn->write_buf + conn->write_start, pending_len);
  conn->write_end = pending_len;
  conn->write_start = 0;
}

// 核心刷出函数：只负责写，绝不关闭连接
// 返回值：0 表示正常（可能发完，可能EAGAIN），-1 表示致命错误（需上层关闭）
int connection_flush(ETcpConnection *conn) {
  while (conn->write_start < conn->write_end) {
    ssize_t n = write(conn->fd, conn->write_buf + conn->write_start,
                      conn->write_end - conn->write_start);
    if (n > 0) {
      conn->write_start += n;
      continue;
    } else if (n < 0) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // 内核缓冲区满，注册 EPOLLOUT，等待下次事件
        channel_enable_writing(&conn->loop->poller, &conn->channel);
        return 0; // 正常情况，不是错误
      } else {
        // 真正的致命错误（EPIPE, ECONNRESET等）
        perror("[FLUSH] write fatal error");
        return -1; // 返回错误，交给上层处理
      }
    }
  }
  // 数据全部发完，重置缓冲区，关闭 EPOLLOUT 监听
  conn->write_end = 0;
  conn->write_start = 0;
  channel_disable_writing(&conn->loop->poller, &conn->channel);
  return 0;
}

// 发送函数：只负责入队 + 条件触发刷出
// 返回值：0 成功，-1 应用层缓冲区满（背压），-2 致命错误
int connection_send(ETcpConnection *conn, const char *data, size_t len) {
  size_t pending = conn->write_end - conn->write_start;
  size_t free_space = BUF_SIZE - pending;
  if (len > free_space) {
    return -1; // 应用层背压
  }
  // 内存整理，将数据移动到开始，尝试扩大尾部空间
  size_t tail_space = BUF_SIZE - conn->write_end;
  if (len > tail_space) {
    connection_compact_write_buffer(conn);
  }
  // 拷贝入队
  memcpy(conn->write_buf + conn->write_end, data, len);
  conn->write_end += len;

  // 仅在缓冲区之前为空时，尝试直接刷出
  // 背压发生时：当第一个包触发write返回EAGAIN后，并监听EPOLLOUT事件
  // pending变为非0，connection_send只拷贝数据，不执行write(大概率返回EAGAIN)，减少系统调用
  if (pending == 0) {
    if (connection_flush(conn) < 0) {
      return -2; // 致命错误，告诉调用者连接已不可用
    }
  }
  return 0;
}
void connection_close(ETcpConnection *conn) {
  poller_del(&conn->loop->poller, &conn->channel);
  close(conn->fd);
  free(conn);
}