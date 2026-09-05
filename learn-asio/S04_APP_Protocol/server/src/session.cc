#include "../include/server.hpp"
#include <asio/detail/socket_ops.hpp>
#include <cstring>
#include <iostream>
#include <ostream>
MsgNode::MsgNode(char *msg, short max_len)
    : total_len(max_len + HEAD_LENGTH), cur_len(0) {
  data = new char[total_len + 1]();
  // 转为网络字节序
   int max_len_host = asio::detail::socket_ops::host_to_network_short(max_len);
  memcpy(data, &max_len_host, HEAD_LENGTH);
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
  ::memset(data_, 0, MAX_LENGTH);
  socket_.async_read_some(asio::buffer(data_, MAX_LENGTH),
                          std::bind(&CSession::handle_read_callback_, this,
                                    std::placeholders::_1,
                                    std::placeholders::_2, shared_self()));
}

void CSession::send(char *msg, int max_length) {
  std::lock_guard<std::mutex> lock(send_lock_);
  int send_que_size = send_que_.size();
  if (send_que_size > MAX_SENDQUE) {
    std::cout << "session: " << uuid_ << " send que fulled, size is " << MAX_SENDQUE
         << std::endl;
    return;
  }

  send_que_.push(std::make_shared<MsgNode>(msg, max_length));
  if (send_que_size > 0) {
    return;
  }
  auto &msgnode = send_que_.front();
  asio::async_write(socket_, asio::buffer(msgnode->data, msgnode->total_len),
                    std::bind(&CSession::handle_write_callback_, this,
                              std::placeholders::_1, shared_self()));
}

void CSession::close() {
  socket_.close();
  b_close_ = true;
}

std::shared_ptr<CSession> CSession::shared_self() { return shared_from_this(); }

void CSession::handle_write_callback_(const asio::error_code &error,
                           std::shared_ptr<CSession> shared_self) {

  if (!error) {
    std::lock_guard<std::mutex> lock(send_lock_);
    std::cout << "send data " << send_que_.front()->data + HEAD_LENGTH << std::endl;
    send_que_.pop();
    if (!send_que_.empty()) {
      auto &msgnode = send_que_.front();
      asio::async_write(socket_,
                        asio::buffer(msgnode->data, msgnode->total_len),
                        std::bind(&CSession::handle_write_callback_, this,
                                  std::placeholders::_1, shared_self));
    }
  } else {
    std::cout << "handle write failed, error is " << error.message() << std::endl;
    close();
    server_->remove_session(uuid_);
  }
}

void CSession::handle_read_callback_(const asio::error_code &error,
                          size_t bytes_transferred,
                          std::shared_ptr<CSession> shared_self) {
  if (!error) {
    // 已经移动的字符数
    int copy_len = 0;
    while (bytes_transferred > 0) {
      if (!b_head_parse_) {
        // 收到的数据不足头部大小
        if (bytes_transferred + recv_head_node_->cur_len < HEAD_LENGTH) {
          memcpy(recv_head_node_->data + recv_head_node_->cur_len,
                 data_ + copy_len, bytes_transferred);
          recv_head_node_->cur_len += bytes_transferred;
          ::memset(data_, 0, MAX_LENGTH);
          socket_.async_read_some(
              asio::buffer(data_, MAX_LENGTH),
              std::bind(&CSession::handle_read_callback_, this, std::placeholders::_1,
                        std::placeholders::_2, shared_self));
          return;
        }
        // 收到的数据比头部多
        // 头部剩余未复制的长度
        int head_remain = HEAD_LENGTH - recv_head_node_->cur_len;
        memcpy(recv_head_node_->data + recv_head_node_->cur_len,
               data_ + copy_len, head_remain);
        // 更新已处理的data长度和剩余未处理的长度
        copy_len += head_remain;
        bytes_transferred -= head_remain;
        // 获取头部数据
        short data_len = 0;
        memcpy(&data_len, recv_head_node_->data, HEAD_LENGTH);
        // 网络字节序转化为本地字节序
        data_len =asio::detail::socket_ops::network_to_host_short(data_len);
        std::cout << "data_len is " << data_len << std::endl;
        // 头部长度非法
        if (data_len > MAX_LENGTH) {
          std::cout << "invalid data length is " << data_len << std::endl;
          server_->remove_session(uuid_);
          return;
        }
        recv_msg_node_ = std::make_shared<MsgNode>(data_len);

        // 消息的长度小于头部规定的长度，说明数据未收全，则先将部分消息放到接收节点里
        if (bytes_transferred < data_len) {
          memcpy(recv_msg_node_->data + recv_msg_node_->cur_len,
                 data_ + copy_len, bytes_transferred);
          recv_msg_node_->cur_len += bytes_transferred;
          ::memset(data_, 0, MAX_LENGTH);
          socket_.async_read_some(
              asio::buffer(data_, MAX_LENGTH),
              std::bind(&CSession::handle_read_callback_, this, std::placeholders::_1,
                        std::placeholders::_2, shared_self));
          // 头部处理完成
          b_head_parse_ = true;
          return;
        }

        memcpy(recv_msg_node_->data + recv_msg_node_->cur_len,
               data_ + copy_len, data_len);
        recv_msg_node_->cur_len += data_len;
        copy_len += data_len;
        bytes_transferred -= data_len;
        recv_msg_node_->data[recv_msg_node_->total_len] = '\0';
        std::cout << "receive data is " << recv_msg_node_->data << std::endl;
        // 此处可以调用Send发送测试
        send(recv_msg_node_->data, recv_msg_node_->total_len);
        // 继续轮询剩余未处理数据
        b_head_parse_ = false;
        recv_head_node_->clear();
        if (bytes_transferred <= 0) {
          ::memset(data_, 0, MAX_LENGTH);
          socket_.async_read_some(
              asio::buffer(data_, MAX_LENGTH),
              std::bind(&CSession::handle_read_callback_, this, std::placeholders::_1,
                        std::placeholders::_2, shared_self));
          return;
        }
        continue;
      }

      // 已经处理完头部，处理上次未接受完的消息数据
      // 接收的数据仍不足剩余未处理的
      int remain_msg = recv_msg_node_->total_len - recv_msg_node_->cur_len;
      if (bytes_transferred < remain_msg) {
        memcpy(recv_msg_node_->data + recv_msg_node_->cur_len,
               data_ + copy_len, bytes_transferred);
        recv_msg_node_->cur_len += bytes_transferred;
        ::memset(data_, 0, MAX_LENGTH);
        socket_.async_read_some(asio::buffer(data_, MAX_LENGTH),
                                std::bind(&CSession::handle_read_callback_, this,
                                          std::placeholders::_1,
                                          std::placeholders::_2, shared_self));
        return;
      }
      memcpy(recv_msg_node_->data + recv_msg_node_->cur_len, data_ + copy_len,
             remain_msg);
      recv_msg_node_->cur_len += remain_msg;
      bytes_transferred -= remain_msg;
      copy_len += remain_msg;
      recv_msg_node_->data[recv_msg_node_->total_len] = '\0';
      std::cout << "receive data is " << recv_msg_node_->data << std::endl;
      // 此处可以调用Send发送测试
      send(recv_msg_node_->data, recv_msg_node_->total_len);
      // 继续轮询剩余未处理数据
      b_head_parse_ = false;
      recv_head_node_->clear();
      if (bytes_transferred <= 0) {
        ::memset(data_, 0, MAX_LENGTH);
        socket_.async_read_some(asio::buffer(data_, MAX_LENGTH),
                                std::bind(&CSession::handle_read_callback_, this,
                                          std::placeholders::_1,
                                          std::placeholders::_2, shared_self));
        return;
      }
      continue;
    }
  } else {
    std::cout << "handle read failed, error is " << error.message() << std::endl;
    close();
    server_->remove_session(uuid_);
  }
}