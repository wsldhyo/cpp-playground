#include "iouring_acceptor.hpp"

#include <cstdlib>

#include "iouring_connection.hpp"
#include "iouring_io_service.hpp"
#include "iouring_request.hpp"
#include "iouring_io_backend.hpp"

void acceptor_init(Acceptor *acceptor, IOService *service) {
  // 1. 创建 listen socket
  int listen_fd = create_socket(PF_INET, SOCK_STREAM, 0);
  struct sockaddr_in addr{};
  bind_and_listen(listen_fd, BACKLOG, &addr, AF_INET, PORT);
  acceptor->service = service;
  acceptor->listen_fd = listen_fd;
}

void acceptor_destroy(Acceptor *acceptor) {
  if (!acceptor)
    return;
  if (acceptor->listen_fd >= 0) {
    close(acceptor->listen_fd);
    acceptor->listen_fd = -1;
  }
}

int acceptor_start(Acceptor *acceptor) {
  IOService *service = acceptor->service;
  Request *req = request_create(OP_ACCEPT);
  if (!req) {
    return -1;
  }
  req->owner.acceptor = acceptor;
  req->service = service;
  req->fd = acceptor->listen_fd;
  int ret = io_backend_prep_accept(&acceptor->service->io_backend, req,
                                   acceptor->listen_fd);
  if (ret < 0) {
    return -1;
  }
  return 0;
}

void acceptor_start_accept(Acceptor *acceptor) {
  Request *req = request_create(OP_ACCEPT);
  if (!req)
    return;
  req->service = acceptor->service;
  req->owner.acceptor = acceptor;
  req->fd = acceptor->listen_fd;
  int ret = io_backend_prep_accept(&acceptor->service->io_backend, req,
                                   acceptor->listen_fd);
  if (ret < 0) {
    request_destroy(req);
    acceptor_destroy(acceptor);
    return;
  }
}

void acceptor_handle_completion(Request *req, int result) {
  Acceptor *acceptor = req->owner.acceptor;
  // accept 失败。
  if (result < 0) {
    int err = -result;
    switch (err) {
    case EINTR:
    case ECONNABORTED:
      acceptor_start(acceptor);
      return;
    default:
      io_service_stop(acceptor->service);
      return;
    }
  }
  // result 是新的 client fd。
  int client_fd = result;
  Connection *conn = connection_create(acceptor->service, client_fd);
  if (!conn) {
    close(client_fd);
    acceptor_start(acceptor);
    return;
  }
  // 启动这个客户端连接的第一次 read。
  connection_start_read(conn);
  acceptor_start(acceptor);
}