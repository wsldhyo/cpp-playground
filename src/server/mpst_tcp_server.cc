#include "mpst_tcp_server.hpp"
#include "common.hpp"
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
/**
 * @brief 回收退出的子进程，避免僵尸进程
 *
 * @param sig 收到的信号编号，通常为SIGCHLD
 */
void reap_child_process(int sig) {
  // 通过循环一次性处理多个同时退出的子进程
  while (waitpid(-1, NULL, WNOWAIT > 0)) {
  }
}

void handle_client_process(int listen_sock_fd, int clnt_fd) {
  pid_t pid = fork();
  if (pid == 0) {
    // 子进程客户端请求
    close(listen_sock_fd);    // 先关闭无用 fd
    handle_clnt_req(clnt_fd); // 处理请求
    close(clnt_fd);
    // exit会执行用户态清理
    _exit(0);
  } else if (pid > 0) {
    // 父进程
    close(clnt_fd); // 关闭无用fd，返回继续监听即可
    return;
  } else {
    // fork 失败
    perror("fork()");
    close(clnt_fd);
  }
}

void handle_clnt_req(int clnt_fd) {
  char buf[BUF_SIZE]{};
  int read_len{0};
  while (true) {
    memset(buf, 0, read_len);
    read_len = read(clnt_fd, buf, BUF_SIZE);
    if (read_len <= 0) {
      break;
    }
    write(clnt_fd, buf, read_len);
  }
  close(clnt_fd);
}