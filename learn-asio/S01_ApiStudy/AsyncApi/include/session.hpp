#ifndef SESSION_HPP
#define SESSION_HPP
#include <asio/error_code.hpp>
#include <cstddef>
#include <memory>
#include <queue>
#include "asio/ip/tcp.hpp"

using asio::ip::tcp;
using sp_sokc_t = std::shared_ptr<tcp::socket>;

constexpr int RECEV_SIZE {1024};

class MsgNode{
public:
    explicit MsgNode(std::size_t total_len);
    MsgNode(char const* msg, std::size_t total_len);
    ~MsgNode();
    char const* data()const;
     std::size_t total_len()const;
     std::size_t cur_len()const;
     void set_curlen(std::size_t len);
private:
    char* msg_;
    std::size_t total_len_;
    std::size_t cur_len_;
};


using sp_msgnode_t = std::shared_ptr<MsgNode>;

// 会话类，管理一个会话
class Session {
public:
  Session(sp_sokc_t sock);
  void connect(const tcp::endpoint& end_point);
  ///socket::async_write_some的回调函数定义（不太正确的定义，消息会乱序）
  /**
    async_write_some的第一个参数是BuffersSequence
    第二个参数是WriteToken，即写回调函数
    回调原型为 void f(asio::error_code, std::size_t)
   */
  void write_callback_err(asio::error_code& ec, std::size_t bytes_tranferred, sp_msgnode_t msgnode);
  void write_to_socket_err(std::string const& buf);
  
  /// 正确的异步发送方式, 通过队列保证异步发送的时序性
  void write_callback(asio::error_code& ec, std::size_t bytes_tranferred);
  void write_to_socket(std::string const& buf);
  
  void write_all_callback(asio::error_code& ec, std::size_t bytes_tranferred);
  void write_all_to_socket(std::string const& buf);

  void read_from_socket();
  void read_callbadk(asio::error_code const&ec, std::size_t bytes_tranferred);
  void read_all_from_socket();
  void read_all_callback(asio::error_code const&ec, std::size_t bytes_tranferred);
private:
    sp_sokc_t socket_;
    sp_msgnode_t send_node_;
    sp_msgnode_t recv_node_;
    std::queue<sp_msgnode_t> send_queue_;
    bool send_pending_;
    bool recv_pending_;
};
#endif