#ifndef IOURING_REQUEST_HPP
#define IOURING_REQUEST_HPP
#include <cstddef> // size_t

#include "iouring_acceptor.hpp"
// struct Proactor;
// struct Connection;
// // 完成回调函数类型
// typedef void (*CompletionCallback)(struct Request *req, int result);
//
// // 操作类型
// typedef enum {
//   OP_ACCEPT,
//   OP_READ,
//   OP_WRITE,
//   // ...
// } OpType;
//
// struct Request {
//   OpType type;
//   struct Proactor *proactor;
//   struct Connection *conn; // 持有引用（增加 conn 的引用计数）
//   int fd;
//   char *buf;
//   size_t len;
//   size_t offset;
//   CompletionCallback complete; // 操作完成时调用
//   // 内部使用：引用计数或链表指针等，如果需要内存池可在此扩展
// };

struct EventLoop;
struct Connection;
struct Acceptor;
typedef enum { OP_ACCEPT, OP_READ, OP_WRITE } OpType;

/*
 * 一次异步 I/O 操作的用户态上下文。
 *
 * 通过 SQE/CQE 的 user_data 与 io_uring 操作关联。
 */
typedef struct Request {
  OpType type;
  struct EventLoop *loop;

  union {
    struct Connection *conn;
    struct Acceptor *acceptor;
  } owner; // Request所属的事件处理器，当Request完成时，将由owner处理I/O结果

  int fd;
  char *buf;
  size_t len;
  size_t offset;

} Request;

Request *request_create(OpType type);
void request_destroy(Request *req);
#endif // IOURING_REQUEST_HPP