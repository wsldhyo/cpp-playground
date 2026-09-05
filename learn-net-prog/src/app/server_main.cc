#include "tcp_server.hpp"
#include "common.hpp"
#include <iostream>
int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "Usage:" << argv[0] << "<port>\n";
    return -1;
  }

  int32_t port;
  str2num(argv[1], port);
  if (!is_vaild_port(port)) {
    std::cerr << "invalid port:" << argv[2] << '\n';
    return 1;
  }
  TcpServer server(port);
  server.start();

  return 0;
}