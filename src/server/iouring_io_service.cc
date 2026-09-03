#include "iouring_io_service.hpp"

#include <cstring>
#include "iouring_acceptor.hpp"
#include "iouring_connection.hpp"
#include "iouring_request.hpp"
int io_service_init(IOService *service) {
  int ret = io_backend_init(&service->io_backend, 256);
  if (ret < 0)
    return ret;
  service->running = 1;
  return 0;
}

void io_service_run(IOService *service) {
  // 首次run，提交准备好的accept请求
  int ret = io_backend_submit(&service->io_backend);
  if (ret < 0)
    return;
  while (service->running) {
    Request *req = NULL;
    int result = 0;
    int ret = io_backend_wait(&service->io_backend, &req, &result);
    if (ret == -EINTR)
      continue;
    if (ret < 0)
      break;
    io_service_dispatch(service, req, result);
    //  把这一轮 dispatch 中产生的所有 SQE 一次性交给内核。
    ret = io_backend_submit(&service->io_backend);

    if (ret < 0)
      break;
  }
}

void io_service_stop(IOService *service) {
  service->running = 0;
}

void io_service_dispatch(IOService *service, Request *req, int result) {
  (void)service;
  switch (req->type) {
  case OP_ACCEPT:
    acceptor_handle_completion(req, result);
    break;
  case OP_RECV:
    connection_handle_read_completion(req->owner.conn, result);
    break;
  case OP_SEND:
    connection_handle_write_completion(req->owner.conn, result);
    break;
  }
  request_destroy(req);
}
void io_service_destroy(IOService *service) {
  io_service_stop(service);
  io_backend_destroy(&service->io_backend);
}