#ifndef MPST_TCP_SERVER_HPP
#define MPST_TCP_SERVER_HPP

/**
 * @brief 回收退出的子进程，避免僵尸进程
 *
 * @param sig 收到的信号编号，通常为SIGCHLD
 */
void reap_child_process(int sig);

void handle_client_process(int listen_sock_fd, int clnt_fd);

/**
 * @brief 处理客户端请求
 *
 * 子进程中负责与单个客户端通信，回显客户端信息
 * @param clnt_fd 客户端连接socket的fd
 */
void handle_clnt_req(int clnt_fd);
#endif // MPST_TCP_SERVER_HPP