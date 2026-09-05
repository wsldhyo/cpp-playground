#include "common.hpp"
#include "tcp_client.hpp"
#include <iostream>
int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <ip> <port>\n";
    return 1;
  }
  int32_t port;
  str2num(argv[2], port);
  if(!is_vaild_port(port)){
    std::cerr << "invalid port:" << argv[2] << '\n';
    return 1;
  }
  TcpClient client(argv[1], static_cast<uint16_t>(port));

  client.run();

  return 0;
}