#ifndef SESSION_HPP
#define SESSION_HPP

#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/system_error.hpp>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>

// 与客户端通信的一个会话类
// 每个连接都将产生一个会话
// Session负责处理一个连接的读和写
class SessionErr {
public:
  explicit SessionErr(asio::io_context &ioc);

  asio::ip::tcp::socket &socket();

  void start();

private:
  // 处理从对端读取数据的回调
  void handle_read_(asio::error_code const &ec, std::size_t bytes_tranferred);
   // 处理向对端写入事件的回调
  void handle_write_(asio::error_code const &ec);

private:
  asio::ip::tcp::socket sock_;
  enum { MAX_LENGTH = 1024 };
  char data_[MAX_LENGTH];
};

class CServer;
class CSession : public std::enable_shared_from_this<CSession> {
public:
  CSession(asio::io_context &ioc, CServer *server);

  asio::ip::tcp::socket &socket();
  std::string &get_uuid();
  void start();

private:
  void handle_read_(asio::error_code const &ec, std::size_t bytes_tranferred,
                    std::shared_ptr<CSession> sp_self);
  void handle_write_(asio::error_code const &ec,
                     std::shared_ptr<CSession> sp_self);

private:
  asio::ip::tcp::socket sock_;
  enum { MAX_LENGTH = 1024 };
  char data_[MAX_LENGTH];
  CServer *server_;
  std::string uuid_;
};

/// 增加发送队列实现全双工通信
class MsgNode {
  friend class CSession2;

public:
  MsgNode(char *msg, int max_len) {
    data = new char[max_len];
    memcpy(data, msg, max_len);
    this->max_len = max_len;
  }
  ~MsgNode() { delete[] data; }
private:
  int cur_len; //
  int max_len;
  char *data;
};
class CServer2;
class CSession2 : public std::enable_shared_from_this<CSession2> {
public:
  CSession2(asio::io_context &ioc, CServer2 *server);

  asio::ip::tcp::socket &socket();
  std::string &get_uuid();
  void start();
  void send(char *msg, int max_length);

private:
  void handle_read_(asio::error_code const &ec, std::size_t bytes_tranferred,
                    std::shared_ptr<CSession2> sp_self);
  void handle_write_(asio::error_code const &ec,
                     std::shared_ptr<CSession2> sp_self);

private:
  asio::ip::tcp::socket sock_;
  enum { MAX_LENGTH = 1024 };
  char data_[MAX_LENGTH];
  CServer2 *server_;
  std::string uuid_;
  std::queue<std::shared_ptr<MsgNode>> send_que_;  // 异步发送队列
  std::mutex send_lock_;
};
#endif