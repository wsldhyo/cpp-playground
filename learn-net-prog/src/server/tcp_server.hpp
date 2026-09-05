
#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP
#include <cstdint>
class TcpServer {
public:
  explicit TcpServer(uint16_t port);
  ~TcpServer();
  void start();

private:
  void create_sock();
  void bind_listen();
  void accept_client();
  void handle_req(int32_t clnt_sock);

private:
  int32_t listen_sock_fd_;
  uint16_t port_;
};
#endif