#ifndef COMMON_HPP
#define COMMON_HPP
#include <cstdint>
#include <netinet/in.h>
constexpr int BUF_SIZE{1024};
constexpr int PORT{8080};
constexpr int BACKLOG{128};

bool str2num(char const *str, int32_t &res);
bool str2numV2(char const *str, int32_t &res);
bool is_vaild_port(int32_t port);
void error_handling(char const *msg);

int create_socket(int domain, int type, int protocol);

struct sockaddr_in;
void bind_and_listen(int listen_sock_fd, int backlog, sockaddr_in *addr,
                     sa_family_t sa_family, int port);

int accept_client(int listen_sock_fd);

#endif