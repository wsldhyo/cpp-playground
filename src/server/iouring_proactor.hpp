#ifndef IOURING_PROACTOR_HPP
#define IOURING_PROACTOR_HPP
#include <liburing.h>
struct Request;
struct Request;

typedef struct Proactor {
  struct io_uring ring;
} Proactor;
int proactor_init(Proactor *p, unsigned entries);
void proactor_destroy(Proactor *p);

int proactor_submit_accept(Proactor *p, Request *req, int listen_fd);

int proactor_submit_read(Proactor *p, Request *req, int fd, void *buf,
                         size_t len);

int proactor_submit_write(Proactor *p, Request *req, int fd, const void *buf,
                          size_t len);

int proactor_wait(Proactor *p, Request **req, int *result);
#endif // IOURING_PROACTOR_HPP