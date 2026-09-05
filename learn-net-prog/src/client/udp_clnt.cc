#include "common.hpp"
#include "udp_clnt.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
UdpClient::UdpClient(const std::string &ip, uint16_t port) {
  sock_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock_ == -1) {
    error_handling("failed to create socket\n");
  }
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_port = htons(port);

  if (inet_pton(AF_INET, ip.c_str(), &server_addr_.sin_addr) != 1)
    error_handling("transform port errorn");
}

UdpClient::~UdpClient() {
  if (sock_ == -1) {
    close(sock_);
  }
}

void UdpClient::run() {
  char buf[BUF_SIZE];

  while (true) {
    std::cout << "Input message(q to quit): ";

    if (fgets(buf, sizeof(buf), stdin) == nullptr)
      break;

    if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n"))
      break;

    sendto(sock_, buf, strlen(buf), 0,
           reinterpret_cast<sockaddr *>(&server_addr_), sizeof(server_addr_));

    sockaddr_in from{};
    socklen_t len = sizeof(from);

    ssize_t n = recvfrom(sock_, buf, sizeof(buf) - 1, 0,
                         reinterpret_cast<sockaddr *>(&from), &len);

    if (n < 0)
      continue;

    buf[n] = '\0';

    std::cout << "Echo: " << buf;
  }
}