#include "udp_server.hpp"
#include "common.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
UdpServer::UdpServer(uint16_t port) : sock_(-1) {
  create_sock();
  bind(port);
}

UdpServer::~UdpServer() {
  if (sock_ != -1)
    close(sock_);
}

void UdpServer::start() {
  char buf[BUF_SIZE];
  while (true) {
    sockaddr_in client{};
    socklen_t len = sizeof(client);

    ssize_t n = recvfrom(sock_, buf, sizeof(buf), 0,
                         reinterpret_cast<sockaddr *>(&client), &len);

    if (n < 0)
      continue;

    sendto(sock_, buf, n, 0, reinterpret_cast<sockaddr *>(&client), len);
  }
}

void UdpServer::create_sock() {
  sock_ = socket(PF_INET, SOCK_DGRAM, 0);
  if (sock_ == -1) {
    error_handling("failed to create socket\n");
  }
}

void UdpServer::bind(uint16_t port) {
  memset(&serv_addr_, 0, sizeof(serv_addr_));
  serv_addr_.sin_family = AF_INET;
  serv_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr_.sin_port = htons(port);
  if (::bind(sock_, reinterpret_cast<sockaddr *>(&serv_addr_),
             sizeof(serv_addr_)) == -1) {
    error_handling("failed to bind socket");
  }
}