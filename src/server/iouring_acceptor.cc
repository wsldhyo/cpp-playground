#include "iouring_acceptor.hpp"

#include <cstdlib>

#include "iouring_connection.hpp"
#include "iouring_eventloop.hpp"
#include "iouring_request.hpp"
#include "iouring_proactor.hpp"

void acceptor_init(Acceptor *acceptor, EventLoop *loop) {
  // 1. 创建 listen socket
  int listen_fd = create_socket(PF_INET, SOCK_STREAM, 0);
  struct sockaddr_in addr{};
  bind_and_listen(listen_fd, BACKLOG, &addr, AF_INET, PORT);
  acceptor->loop = loop;
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

void acceptor_start(Acceptor *acceptor) {
  EventLoop *loop = acceptor->loop;
  Request *req = request_create(OP_ACCEPT);
  if (!req) {
    return;
  }
  req->owner.acceptor = acceptor;
  req->loop = loop;
  req->fd = acceptor->listen_fd;
  int ret = proactor_submit_accept(&acceptor->loop->proactor, req,
                                   acceptor->listen_fd);
  if (ret < 0) {
    request_destroy(req);
    return;
  }
}

void acceptor_handle_completion(Request *req, int result) {
  Acceptor *acceptor = req->owner.acceptor;

  /*
   * 无论成功失败，
   * 当前这次 Request 都结束了。
   */
  request_destroy(req);

  /*
   * accept 失败。
   */
  if (result < 0) {

    /*
     * 可以根据错误类型决定
     * 是否继续 accept。
     */
    acceptor_start(acceptor);
    return;
  }

  /*
   * result 是新的 client fd。
   */
  int client_fd = result;

  Connection *conn = connection_create(acceptor->loop, client_fd);

  if (!conn) {
    close(client_fd);

    /*
     * 继续接收新的连接。
     */
    acceptor_start(acceptor);
    return;
  }

  /*
   * 启动这个客户端连接的第一次 read。
   */
  connection_start_read(conn);

  /*
   * accept 是持续进行的。
   *
   * 当前连接建立后，
   * 再提交下一次 accept。
   */
  acceptor_start(acceptor);
}