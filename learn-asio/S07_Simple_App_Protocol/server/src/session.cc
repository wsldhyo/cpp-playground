#include "../include/server.hpp"
#include <asio/buffer.hpp>
#include <asio/detail/socket_ops.hpp>
#include <asio/read.hpp>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include <sys/socket.h>
MsgNode::MsgNode(char *msg, short max_len)
    : total_len(max_len + HEAD_LENGTH), cur_len(0) {
  data = new char[total_len + 1]();
  // 转为网络字节序
  //int max_len_host = asio::detail::socket_ops::host_to_network_short(max_len);
  memcpy(data, &max_len, HEAD_LENGTH);
  memcpy(data + HEAD_LENGTH, msg, max_len);
  data[total_len] = '\0';
}

MsgNode::MsgNode(short max_len) : total_len(max_len), cur_len(0) {
  data = new char[total_len + 1]();
}

MsgNode::~MsgNode() { delete[] data; }

void MsgNode::clear() {
  ::memset(data, 0, total_len);
  cur_len = 0;
}

CSession::CSession(asio::io_context &io_context, CServer *server)
    : socket_(io_context), server_(server), b_close_(false),
      b_head_parse_(false) {
  boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
  uuid_ = boost::uuids::to_string(a_uuid);
  recv_head_node_ = std::make_shared<MsgNode>(HEAD_LENGTH);
}
CSession::~CSession() { std::cout << "~CSession destruct" << std::endl; }

tcp::socket &CSession::get_socket() { return socket_; }

std::string &CSession::get_uuid() { return uuid_; }

void CSession::start() {
  recv_head_node_->clear();

  asio::async_read(socket_, asio::buffer(recv_head_node_->data, HEAD_LENGTH),
                   std::bind(&CSession::read_head_callback_, this,
                             std::placeholders::_1, std::placeholders::_2,
                             shared_self()));
}

void CSession::send(char *msg, int max_length) {
  std::lock_guard<std::mutex> lock(send_lock_);
  int send_que_size = send_que_.size();
  if (send_que_size > MAX_SENDQUE) {
    std::cout << "session: " << uuid_ << " send que fulled, size is "
              << MAX_SENDQUE << std::endl;
    return;
  }

  send_que_.push(std::make_shared<MsgNode>(msg, max_length));
  if (send_que_size > 0) {
    return;
  }
  auto &msgnode = send_que_.front();
  asio::async_write(socket_, asio::buffer(msgnode->data, msgnode->total_len),
                    std::bind(&CSession::write_callback_, this,
                              std::placeholders::_1, shared_self()));
}

void CSession::close() {
  socket_.close();
  b_close_ = true;
}

std::shared_ptr<CSession> CSession::shared_self() { return shared_from_this(); }

void CSession::write_callback_(const asio::error_code &error,
                                      std::shared_ptr<CSession> shared_self) {

  if (!error) {
    std::lock_guard<std::mutex> lock(send_lock_);
    std::cout << "send data " << send_que_.front()->data + HEAD_LENGTH
              << std::endl;
    send_que_.pop();
    if (!send_que_.empty()) {
      auto &msgnode = send_que_.front();
      asio::async_write(socket_,
                        asio::buffer(msgnode->data, msgnode->total_len),
                        std::bind(&CSession::write_callback_, this,
                                  std::placeholders::_1, shared_self));
    }
  } else {
    std::cout << "handle write failed, error is " << error.message()
              << std::endl;
    close();
    server_->remove_session(uuid_);
  }
}

void CSession::read_head_callback_(const asio::error_code &error,
                                   size_t bytes_transferred,
                                   std::shared_ptr<CSession> shared_self) {
  if (!error) {
    if (bytes_transferred < HEAD_LENGTH) {
      std::cout << "read head length error\n";
      close();
      server_->remove_session(uuid_);
      return;
    }
    short data_len = 0;
    memcpy(&data_len, recv_head_node_->data, HEAD_LENGTH);
    std::cout << "recv data length is " << data_len << std::endl;

    if (data_len > MAX_LENGTH) {
      std::cout << "invalid data length" << std::endl;
      server_->remove_session(uuid_);
    }

    recv_msg_node_ = std::make_shared<MsgNode>(data_len);
    asio::async_read(
        socket_, asio::buffer(recv_msg_node_->data, recv_msg_node_->total_len),
        std::bind(&CSession::read_msg_callback_, this, std::placeholders::_1,
                  std::placeholders::_2, shared_self));
  } else {
    std::cout << "read head info failed, error is " << error.message() << std::endl;
    close();
    server_->remove_session(uuid_);
  }
}

void CSession::read_msg_callback_(const asio::error_code &error,
                                  size_t bytes_transferred,
                                  std::shared_ptr<CSession> shared_self) {
  if (!error) {
     recv_msg_node_->data[recv_msg_node_->total_len] = '\0';
     std::cout << "recv msg is " << recv_msg_node_->data << std::endl;
     // 回传给客户端
     send(recv_msg_node_->data, recv_msg_node_->total_len);

     // 挂起读头部事件，准备读取下一消息
     recv_head_node_->clear();
     asio::async_read(socket_, asio::buffer(recv_head_node_->data, HEAD_LENGTH),
      std::bind(&CSession::read_head_callback_, this, std::placeholders::_1, std::placeholders::_2, shared_self));   
  } else {
    std::cout << "read head info failed, error is " << error.message() << std::endl;
    close();
    server_->remove_session(uuid_);
  }
}