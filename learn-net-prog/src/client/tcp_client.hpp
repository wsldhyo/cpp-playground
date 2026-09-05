#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP
#include <cstdint>
#include <string>
class TcpClient {
public:
  TcpClient(std::string ip, uint16_t port);
  ~TcpClient();
  void run();

private:
  void connect_server();
  void loop();
  void send_msg(char const* msg, size_t const len);
  void recv_msg(char * msg, size_t const len);

private:
  int sock_;
  std::string ip_;
  uint16_t port_;
};
#endif