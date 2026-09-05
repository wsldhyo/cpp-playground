#include "common.hpp"
#include <cctype>
#include <cerrno>
#include <charconv>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

bool str2num(char const *str, int32_t &res) {
  if (!str)
    return false;

  bool neg = false;
  int64_t val = 0;

  // sign
  if (*str == '-') {
    neg = true;
    ++str;
  } else if (*str == '+') {
    ++str;
  }

  if (*str == '\0')
    return false;

  while (*str) {
    if (*str < '0' || *str > '9')
      return false;

    val = val * 10 + (*str - '0');

    // overflow check
    if (!neg && val > INT32_MAX)
      return false;
    if (neg && -val < INT32_MIN)
      return false;

    ++str;
  }

  res = neg ? -static_cast<int32_t>(val) : static_cast<int32_t>(val);

  return true;
}

bool str2numV2(char const *str, int32_t &res) {
  if (!str)
    return false;

  // skip leading spaces
  while (*str && std::isspace(static_cast<unsigned char>(*str)))
    ++str;

  const char *end = str + std::strlen(str);

  auto [ptr, ec] = std::from_chars(str, end, res);

  if (ec != std::errc()) {
    return false;
  }

  // skip trailing spaces
  while (ptr < end && std::isspace(static_cast<unsigned char>(*ptr)))
    ++ptr;

  return ptr == end;
}

bool is_vaild_port(int32_t port) { return port <= 65535; }

void error_handling(const char *msg) {
  printf("%s, errno:%d", msg, errno);
  exit(-1);
}

int create_socket(int domain, int type, int protocol) {
  int listen_sock_fd = socket(PF_INET, SOCK_STREAM, 0);
  if (listen_sock_fd < 0) {
    perror("listen()");
    error_handling("failed to create socket");
  }
  return listen_sock_fd;
}

void bind_and_listen(int listen_sock_fd, int backlog, sockaddr_in *addr,
                     sa_family_t sa_family, int port) {
  if (addr == nullptr) {
    printf("invaild addr argument in bind_and_listen func\n");
    exit(-1);
  }
  // 填充服务端地址信息，用于 bind()
  memset(addr, 0, sizeof(sockaddr_in));

  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = htonl(INADDR_ANY); // 监听本机所有网卡
  addr->sin_port = htons(port);

  // 将监听套接字绑定到指定 IP 和端口
  if (bind(listen_sock_fd, (sockaddr *)addr, sizeof(sockaddr_in)) == -1) {
    perror("bind()");
    error_handling("failed to bind socket");
  }

  // 将套接字转换为监听状态，等待客户端连接
  if (listen(listen_sock_fd, backlog) == -1) {
    perror("listen()");
    error_handling("listen socket error");
  }
}

int accept_client(int listen_sock_fd) {
  while (true) {
    // 对端地址，每个新连接都独一份，在循环里创建即可
    sockaddr_in clnt_addr{};
    socklen_t clnt_addr_len = sizeof(clnt_addr);
    int clnt_fd =
        accept(listen_sock_fd, reinterpret_cast<sockaddr *>(&clnt_addr),
               &clnt_addr_len);

    if (clnt_fd >= 0)
      return clnt_fd;

    if (errno == EINTR)
      continue;

    perror("accept()");
    return -1;
  }
}

bool writelen(int clnt_fd, const char *buf, size_t len) {
  size_t total = 0;
  while (total < len) {
    ssize_t n = write(clnt_fd, buf + total, len - total);

    if (n <= 0) {
      return false;
    }

    total += static_cast<size_t>(n);
  }

  return true;
}

int set_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1)
    return -1;
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}