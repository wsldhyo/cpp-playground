#include "tcp_client.hpp"
#include "common.hpp"
#include <arpa/inet.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

TcpClient::TcpClient(std::string ip, uint16_t port)
    : sock_(-1), ip_(std::move(ip)), port_(port) {
  connect_server();
}

TcpClient::~TcpClient() {
  if (sock_ != -1)
    close(sock_);
}

void TcpClient::connect_server() {
  // 创建 TCP 套接字
  sock_ = socket(PF_INET, SOCK_STREAM, 0);
  if (sock_ == -1) {

    error_handling("failed to create socket\n");
  }
  // 填充服务器地址
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip_.c_str());
  addr.sin_port = htons(port_);
  // 发起 TCP 连接请求 
  if (connect(sock_, (sockaddr *)&addr, sizeof(addr)) == -1) {
    error_handling("failed to connect server\n");
  } else {
    std::cout << "connect server success\n";
  }
}

void TcpClient::run() {
  char msg[BUF_SIZE]{};
  while (true) {
    std::cout << "Input message(Q to quit): ";
    // 读取用户输入
    fgets(msg, sizeof(msg), stdin);

    if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")){
      break;
    }
    // 发送给服务器
    size_t msg_len = strlen(msg);
    send_msg(msg, msg_len);
    // 原样读取
    recv_msg(msg, msg_len);
    std::cout << "msg from server: " << msg;
  }
  
  
  // 通知服务端关闭链接
  shutdown(sock_, SHUT_WR);

  // 接收滞留在网络中的数据 
  // 一般很少
  recv_msg(msg, BUF_SIZE);
}

void TcpClient::send_msg(char const *msg, size_t const len) {

  size_t total = 0;

  // write() 可能一次无法发送全部数据，因此循环发送
  while (total < len) {
    ssize_t n = write(sock_, msg + total, len - total);

    if (n <= 0)
      error_handling("write");

    total += n;
  }
}

void TcpClient::recv_msg(char *msg, size_t const len) {
  size_t total = 0;
  // 按照发送长度接收完整消息，read()可能无法一次完全读取，使用循环读取
  while (total < len) {
    ssize_t n = read(sock_, msg + total, len - total);

    if (n == 0) { // read 返回 0，表示对端关闭连接
      std::cout << "server closed\n";
      exit(1);
    }

    if (n < 0) {
      error_handling("read");
    }

    total += static_cast<size_t>(n);
  }

  msg[total] = '\0';
}
