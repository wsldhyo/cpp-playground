#include "../include/session.hpp"
#include <asio/detail/io_control.hpp>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <type_traits>

MsgNode::MsgNode(std::size_t total_len) : total_len_(total_len), cur_len_(0) {
  msg_ = new char[total_len];
}

MsgNode::MsgNode(char const *msg, std::size_t total_len) : MsgNode(total_len) {
  memcpy(msg_, msg, total_len);
}

MsgNode::~MsgNode() { delete[] msg_; }

char const *MsgNode::data() const { return msg_; }
std::size_t MsgNode::total_len() const { return total_len_; }
std::size_t MsgNode::cur_len() const { return cur_len_; }
void MsgNode::set_curlen(std::size_t len) { cur_len_ = len; }
Session::Session(sp_sokc_t sock)
    : socket_(sock), send_pending_(false), recv_pending_(false) {}
void Session::connect(const tcp::endpoint &end_point) {
  socket_->connect(end_point);
}
void Session::write_callback_err(asio::error_code &ec,
                                 std::size_t bytes_tranferred,
                                 sp_msgnode_t msgnode) {

  // 前一次async_write_some发送长度bytes_transferred
  // 和前n次async_write_some发送长度cur_len
  // 小于总长度，则继续调用async_write_some发送数据
  if (bytes_tranferred + msgnode->cur_len() < msgnode->total_len()) {
    // 更新已发送长度
    send_node_->set_curlen(send_node_->cur_len() + bytes_tranferred);

    this->socket_->async_write_some(
        asio::buffer(send_node_->data() + send_node_->cur_len(),
                     send_node_->total_len() - send_node_->cur_len()),
        std::bind(&Session::write_callback_err, this, std::placeholders::_1,
                  std::placeholders::_2, send_node_));
  }
}

void Session::write_to_socket_err(std::string const &buf) {
  send_node_ = std::make_shared<MsgNode>(buf.c_str(), buf.length());

  // 绑定asio异步写方法的回调函数
  this->socket_->async_write_some(
      asio::buffer(send_node_->data(), send_node_->total_len()),
      std::bind(&Session::write_callback_err, this, std::placeholders::_1,
                std::placeholders::_2, send_node_));
}
void Session::write_callback(asio::error_code &ec,
                             std::size_t bytes_transferred) {
  if (ec.value() != 0) {
    std::cout << "Error code is " << ec.value() << "Message is " << ec.message()
              << std::endl;
    return;
  }
  auto &send_data = send_queue_.front();
  send_data->set_curlen(send_data->cur_len() + bytes_transferred);
  if (send_data->cur_len() < send_data->total_len()) {
    this->socket_->async_write_some(
        asio::buffer(send_data->data() + send_data->cur_len(),
                     send_data->total_len() - send_data->cur_len()),
        std::bind(&Session::write_callback, this, std::placeholders::_1,
                  std::placeholders::_2));
    return;
  }
  send_queue_.pop();
  if (send_queue_.empty()) {
    send_pending_ = false;
  } else {
    // 做好数据偏移
    auto &send_data = send_queue_.front();
    this->socket_->async_write_some(
        asio::buffer(send_data->data() + send_node_->cur_len(),
                     send_data->total_len() - send_data->cur_len()),
        std::bind(&Session::write_callback, this, std::placeholders::_1,
                  std::placeholders::_2));
  }
}
void Session::write_to_socket(std::string const &buf) {
  send_queue_.emplace(std::make_shared<MsgNode>(buf.c_str(), buf.length()));
  if (send_pending_) {
    return;
  }
  this->socket_->async_write_some(asio::buffer(buf),
                                  std::bind(&Session::write_callback, this,
                                            std::placeholders::_1,
                                            std::placeholders::_2));
  send_pending_ = true;
}

void Session::write_all_callback(asio::error_code &ec,
                                 std::size_t bytes_tranferred) {
  if (ec.value() != 0) {
    std::cout << "Error occured! Error code = " << ec.value()
              << "Message :" << ec.message() << std::endl;
    return;
  }
  send_queue_.pop();
  if (send_queue_.empty()) {
    send_pending_ = false;
  } else {
    // 做好数据偏移
    auto &send_data = send_queue_.front();
    this->socket_->async_write_some(
        asio::buffer(send_data->data() + send_node_->cur_len(),
                     send_data->total_len() - send_data->cur_len()),
        std::bind(&Session::write_all_callback, this, std::placeholders::_1,
                  std::placeholders::_2));
  }
}
void Session::write_all_to_socket(std::string const &buf) {
  send_queue_.emplace(std::make_shared<MsgNode>(buf.c_str(), buf.length()));
  if (send_pending_) {
    return;
  }
  this->socket_->async_send(asio::buffer(buf),
                            std::bind(&Session::write_all_callback, this,
                                      std::placeholders::_1,
                                      std::placeholders::_2));
  send_pending_ = true;
}
void Session::read_from_socket() {
  if (recv_pending_) {
    return;
  }
  recv_node_ = std::make_shared<MsgNode>(RECEV_SIZE);
  socket_->async_read_some(
      asio::buffer(recv_node_->data(), recv_node_->total_len()),
      std::bind(&Session::read_callbadk, this, std::placeholders::_1,
                std::placeholders::_2));
  recv_pending_ = true;
}

void Session::read_callbadk(asio::error_code const &ec,
                            std::size_t bytes_tranferred) {
  recv_node_->set_curlen(recv_node_->cur_len() + bytes_tranferred);
  if (recv_node_->cur_len() < recv_node_->total_len()) {
    socket_->async_read_some(
        asio::buffer(recv_node_->data() + recv_node_->cur_len(),
                     recv_node_->total_len() - recv_node_->cur_len()),
        std::bind(&Session::read_callbadk, this, std::placeholders::_1,
                  std::placeholders::_2));
    return;
  }
}

void Session::read_all_from_socket() {
  if (recv_pending_) {
    return;
  }
  recv_node_ = std::make_shared<MsgNode>(RECEV_SIZE);
  socket_->async_receive(asio::buffer(recv_node_->data(), recv_node_->total_len()),
  std::bind(&Session::read_all_callback, this, std::placeholders::_1, std::placeholders::_2));
}
void Session::read_all_callback(asio::error_code const &ec,
                                std::size_t bytes_tranferred) {
  recv_node_->set_curlen(recv_node_->cur_len() + bytes_tranferred);
  recv_node_ = nullptr;
  recv_pending_ = false;
}