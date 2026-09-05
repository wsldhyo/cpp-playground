#ifndef IOURING_REQUEST_HPP
#define IOURING_REQUEST_HPP
#include <cstddef> // size_t

#include "iouring_acceptor.hpp"
struct IOService;
struct Connection;
struct Acceptor;
typedef enum { OP_ACCEPT, OP_SEND, OP_RECV } OpType;
// 一次异步 I/O 操作的用户态上下文。
typedef struct Request {
  OpType type;
  struct IOService *service;
  // Request所属的Completion Handler，
  // 当Request完成时，将由owner处理I/O结果
  union {
    struct Connection *conn;
    struct Acceptor *acceptor;
  } owner;
  int fd;
  // 根据I/O类型，指向Connection的读缓冲区或写缓冲区
  char *buf;
  size_t len;
} Request;

Request *request_create(OpType type);
void request_destroy(Request *req);
#endif // IOURING_REQUEST_HPP