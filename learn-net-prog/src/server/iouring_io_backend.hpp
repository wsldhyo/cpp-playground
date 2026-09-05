#ifndef IOURING_IO_BACKEND_HPP
#define IOURING_IO_BACKEND_HPP
#include <liburing.h>
struct Request;
struct Request;

typedef struct IOBackend {
  struct io_uring ring;
  unsigned prepared_sqes; // 当前已准备未提交的SQE数量
} IOBackend;
int io_backend_init(IOBackend *p, unsigned entries);
void io_backend_destroy(IOBackend *p);

int io_backend_prep_accept(IOBackend *p, Request *req, int listen_fd);
int io_backend_prep_send(IOBackend *p, Request *req, int fd, const void *buf,
                         size_t len);
int io_backend_prep_recv(IOBackend *p, Request *req, int fd, void *buf,
                         size_t len);
int io_backend_submit(IOBackend *p);

int io_backend_wait(IOBackend *p, struct Request **req, int *result);

// 批量消费
int io_backend_wait_batch(IOBackend *p, Request **reqs, int *results,
                          unsigned max, unsigned *count);
#endif // IOURING_IO_BACKEND_HPP