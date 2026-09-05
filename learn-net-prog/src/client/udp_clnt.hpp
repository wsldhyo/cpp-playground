#ifndef UDP_CLNT_HPP
#define UDP_CLNT_HPP
#include <cstdint>
#include <netinet/in.h>
#include <string>
class UdpClient {
public:
  UdpClient(const std::string &ip, uint16_t port);

  ~UdpClient();

  void run();

private:
  int sock_{-1};
  sockaddr_in server_addr_{};
};

#endif // UDP_CLNT_HPP