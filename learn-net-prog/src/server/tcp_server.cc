#include "tcp_server.hpp"
#include "common.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

TcpServer::TcpServer(uint16_t port) : listen_sock_fd_(-1), port_(port) {}

// 关闭监听套接字，释放系统资源
TcpServer::~TcpServer() { close(listen_sock_fd_); }

void TcpServer::start() {
  // 创建监听套接字
  create_sock();

  // 绑定地址并开始监听
  bind_listen();

  // 接收客户端连接
  accept_client();
}

void TcpServer::create_sock() {
  // 创建 TCP 监听套接字
  listen_sock_fd_ = socket(PF_INET, SOCK_STREAM, 0);
  if (listen_sock_fd_ == -1) {
    error_handling("failed to create socket\n");
  }
}

void TcpServer::bind_listen() {
  // 填充服务端地址信息，用于 bind()
  sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听本机所有网卡
  serv_addr.sin_port = htons(port_);

  // 将监听套接字绑定到指定 IP 和端口
  if (bind(listen_sock_fd_, (sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
    error_handling("failed to bind socket\n");
  }

  // 将套接字转换为监听状态，等待客户端连接
  if (listen(listen_sock_fd_, 5) == -1) {
    error_handling("listen socket error\n");
  }
}

void TcpServer::accept_client() {
  int32_t clnt_sock{-1};

  sockaddr clnt_addr;
  socklen_t clnt_addr_len = sizeof(clnt_addr);

  // 连续处理 5 个客户端
  for (int32_t i = 0; i < 5; ++i) {
    // 阻塞等待客户端建立连接
    clnt_sock = accept(listen_sock_fd_, (sockaddr *)&clnt_addr, &clnt_addr_len);

    if (clnt_sock == -1) {
      error_handling("accept peer socket error\n");
    } else {
      std::cout << "connected client " << i + 1 << '\n';
    }

    // 处理该客户端请求（回显服务器）
    handle_req(clnt_sock);

    // 客户端断开后关闭对应连接
    close(clnt_sock);
  }
}

void TcpServer::handle_req(int32_t clnt_sock) {
  ssize_t msg_len = 0;
  char msg[BUF_SIZE]{};

  while ((msg_len = read(clnt_sock, msg, BUF_SIZE)) > 0) {
    if (msg_len == 0) {
      std::cout << "client closed connection\n";
      break;
    }
    if (msg_len < 0) {
      error_handling("read");
    }
    write(clnt_sock, msg, msg_len);
  }
}
