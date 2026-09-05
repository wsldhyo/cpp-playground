#include "iouring_io_backend.hpp"

#include <cerrno>
#include <cstring>

#include "iouring_request.hpp"

int io_backend_init(IOBackend *p, unsigned entries) {
  memset(p, 0, sizeof(*p));
  int ret = io_uring_queue_init(entries, &p->ring, 0);
  if (ret == 0)
    p->prepared_sqes = 0;
  return ret;
}

void io_backend_destroy(IOBackend *p) { io_uring_queue_exit(&p->ring); }

struct io_uring_sqe *io_backend_get_sqe(IOBackend *p) {
  struct io_uring_sqe *sqe;
  sqe = io_uring_get_sqe(&p->ring);
  if (sqe)
    return sqe;
  // SQ 当前没有空间。先把已经准备好的 SQE 提交给内核，
  int ret = io_uring_submit(&p->ring);
  if (ret < 0)
    return NULL;
  if (ret > 0)
    p->prepared_sqes -= (unsigned)ret;
  // 释放 SQ 空间后，再次尝试获取。
  return io_uring_get_sqe(&p->ring);
}

int io_backend_prep_accept(IOBackend *p, Request *req, int listen_fd) {
  struct io_uring_sqe *sqe = io_backend_get_sqe(p);
  if (!sqe)
    return -ENOSPC;
  // 不关心客户端地址， 所以 addr / addrlen 都传 NULL。
  io_uring_prep_accept(sqe, listen_fd, NULL, NULL, 0);
  io_uring_sqe_set_data(sqe, req);
  p->prepared_sqes++;
  return 0;
}

int io_backend_prep_send(IOBackend *p, Request *req, int fd, const void *buf,
                         size_t len) {
  struct io_uring_sqe *sqe = io_backend_get_sqe(p);
  if (!sqe)
    return -ENOSPC;
  io_uring_prep_send(sqe, fd, buf, len, 0);
  io_uring_sqe_set_data(sqe, req);
  p->prepared_sqes++;
  return 0;
}

int io_backend_prep_recv(IOBackend *p, Request *req, int fd, void *buf,
                         size_t len) {
  struct io_uring_sqe *sqe = io_backend_get_sqe(p);
  if (!sqe)
    return -ENOSPC;
  io_uring_prep_recv(sqe, fd, buf, len, 0);
  io_uring_sqe_set_data(sqe, req);
  p->prepared_sqes++;

  return 0;
}

int io_backend_submit(IOBackend *p) {
  if (p->prepared_sqes == 0)
    return 0;
  int ret = io_uring_submit(&p->ring);
  if (ret < 0) {
    // 提交失败时，调用方不要立即销毁 Request。
    // 因为这些 Request 仍然可能对应 SQ 中尚未提交的 SQE。
    return ret;
  }
  p->prepared_sqes -= (unsigned)ret;
  return ret;
}

int io_backend_wait(IOBackend *p, struct Request **req, int *result) {
  struct io_uring_cqe *cqe;
  int ret = io_uring_wait_cqe(&p->ring, &cqe);
  if (ret < 0)
    return ret;
  *req = (Request *)io_uring_cqe_get_data(cqe);
  *result = cqe->res;
  // CQE 已经被消费。
  io_uring_cqe_seen(&p->ring, cqe);
  return 0;
}

// 批量消费
int io_backend_wait_batch(IOBackend *p, Request **reqs, int *results,
                          unsigned max, unsigned *count) {
  if (max == 0)
    return -EINVAL;
  struct io_uring_cqe *cqe;
  // 至少等待一个 CQE。
  int ret = io_uring_wait_cqe(&p->ring, &cqe);
  if (ret < 0)
    return ret;

  // 把当前已经完成的 CQE 尽可能批量拿出来。
  struct io_uring_cqe *cqes[max];
  unsigned nr = io_uring_peek_batch_cqe(&p->ring, cqes, max);
  for (unsigned i = 0; i < nr; ++i) {
    reqs[i] = (Request *)io_uring_cqe_get_data(cqes[i]);
    results[i] = cqes[i]->res;
  }
  // 一次性推进 CQ head， 相当于批量消费这些 CQE。
  io_uring_cq_advance(&p->ring, nr);
  *count = nr;
  return 0;
}