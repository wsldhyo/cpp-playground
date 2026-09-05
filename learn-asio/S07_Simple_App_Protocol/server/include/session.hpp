#ifndef SESSION_HPP
#define SESSION_HPP
#include "const.h"
#include <asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <memory>
#include <mutex>
#include <queue>

using asio::ip::tcp;
class CServer;

struct MsgNode {
  MsgNode(char *msg, short max_len);

  MsgNode(short max_len);

  ~MsgNode();

  void clear();

  short cur_len;
  short total_len;
  char *data;
};

class CSession : public std::enable_shared_from_this<CSession> {
public:
  CSession(asio::io_context &io_context, CServer *server);
  ~CSession();
  tcp::socket &get_socket();
  std::string &get_uuid();
  void start();
  void send(char *msg, int max_length);
  void close();
  std::shared_ptr<CSession> shared_self();

private:
  void read_head_callback_(const asio::error_code &error, size_t bytes_transferred,
                  std::shared_ptr<CSession> shared_self);
  void read_msg_callback_(const asio::error_code &error, size_t bytes_transferred,
                  std::shared_ptr<CSession> shared_self);
  void write_callback_(const asio::error_code &error,
                   std::shared_ptr<CSession> shared_self);
  tcp::socket socket_;
  std::string uuid_;
  char data_[MAX_LENGTH];
  CServer *server_;
  bool b_close_;
  std::queue<std::shared_ptr<MsgNode>> send_que_;
  std::mutex send_lock_;
  // 收到的消息结构
  std::shared_ptr<MsgNode> recv_msg_node_;
  bool b_head_parse_;
  // 收到的头部结构
  std::shared_ptr<MsgNode> recv_head_node_;
};
#endif