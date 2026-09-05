#include "udp_clnt.hpp"
#include <iostream>
int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " <IP> <port>\n";
    exit(1);
  }
  UdpClient client(argv[1], std::stoi(argv[2]));
  
  client.run();
  return 0;
}