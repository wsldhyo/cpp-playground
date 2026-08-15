#ifndef EPOLL_TCP_CONNECTION_HPP
#define EPOLL_TCP_CONNECTION_HPP
#include "common.hpp"
#include "epoll_tcp_channel.hpp"
#include <cstring>
#include <sys/types.h>
struct EventLoop;
typedef struct {
  int fd;
  ETcpChannel channel;
  EventLoop *loop;
  //    // 输入缓冲区
  char read_buf[BUF_SIZE];
  size_t read_len; // 本次 read() 实际读到的数据长度
  // 输出缓冲区
  /*
   * write_buf:
   *
   * 0                write_start          write_end
   * |--------------------|---------------------|-----------|
   *      已发送                未发送             空闲
   */
  char write_buf[BUF_SIZE];
  size_t write_start; // 下一个待发送数据的位置
  size_t write_end;   // 写缓冲区有效数据的末尾位置
  size_t remain_len;  // 上次写由于缓冲区满，而未写入的字节数
  // 是否因为输出缓冲区满而暂停应用层读取
  int read_paused;
} ETcpConnection;

ETcpConnection *connection_create(EventLoop *loop, int fd);

void connection_handle_read(ETcpChannel *etch);

int connection_check_space(ETcpConnection *conn);

void connection_handle_write(ETcpChannel *etch);

void connection_handle_error(ETcpChannel *etch);

int connection_send(ETcpConnection *conn, const char *data, size_t len);

void connection_compact_write_buffer(ETcpConnection *conn);

int connection_flush(ETcpConnection *conn); 

void connection_close(ETcpConnection *conn);

void connection_after_event(ETcpChannel *etch);

#endif // EPOLL_TCP_CONNECTION_HPP