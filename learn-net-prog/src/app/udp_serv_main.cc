#include "udp_server.hpp"
#include <iostream>
#include <netinet/in.h>
int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "Usage:" << argv[0] << "<port>\n";
    exit(1);
  }
  UdpServer serv(atoi(argv[1]));
  serv.start();
  return 0;
}