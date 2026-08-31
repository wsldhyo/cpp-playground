#include "iouring_proactor.hpp"

#include <cerrno>

#include "iouring_request.hpp"

int proactor_init(Proactor *p, unsigned entries) {
  return io_uring_queue_init(entries, &p->ring, 0);
}

void proactor_destroy(Proactor *p) { io_uring_queue_exit(&p->ring); }

int proactor_submit_accept(Proactor *p, Request *req, int listen_fd) {
  struct io_uring_sqe *sqe;
  sqe = io_uring_get_sqe(&p->ring);
  if (!sqe)
    return -ENOSPC;
  /*
   * 本例不关心客户端地址，
   * 所以 addr / addrlen 都传 NULL。
   */
  io_uring_prep_accept(sqe, listen_fd, NULL, NULL, 0);
  /*
   * CQE 完成后，通过 user_data
   * 找回 Request。
   */
  io_uring_sqe_set_data(sqe, req);
  return io_uring_submit(&p->ring);
  // TODO 外部释放Req？
  // if (ret < 0) {
  //   request_destroy(req);
  // }
}

int proactor_submit_read(Proactor *p, Request *req, int fd, void *buf,
                         size_t len) {
  struct io_uring_sqe *sqe;
  sqe = io_uring_get_sqe(&p->ring);
  if (!sqe)
    return -ENOSPC;
  io_uring_prep_recv(sqe, fd, buf, len, 0);
  io_uring_sqe_set_data(sqe, req);
  return io_uring_submit(&p->ring);
}

int proactor_submit_write(Proactor *p, struct Request *req, int fd,
                          const void *buf, size_t len) {
  struct io_uring_sqe *sqe;
  sqe = io_uring_get_sqe(&p->ring);
  if (!sqe  )
    return -ENOSPC;
  io_uring_prep_send(sqe, fd, buf, len, 0);
  io_uring_sqe_set_data(sqe, req);
  return io_uring_submit(&p->ring);
}

int proactor_wait(Proactor *p, struct Request **req, int *result) {
  struct io_uring_cqe *cqe;
  int ret = io_uring_wait_cqe(&p->ring, &cqe);
  if (ret < 0)
    return ret;
  *req = (Request*)io_uring_cqe_get_data(cqe);
  *result = cqe->res;
  /*
   * CQE 已经被我们消费。
   */
  io_uring_cqe_seen(&p->ring, cqe);
  return 0;
}