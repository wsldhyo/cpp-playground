#include <asio/awaitable.hpp>
#include <asio/buffer.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/error_code.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>

#include "../include/server.hpp"
#include "../include/session.hpp"
#include "../include/logic_system.hpp"
Session::Session(asio::io_context &_io_context, Server *_server)
    : io_context_(_io_context), server_(_server), socket_(_io_context), b_close_(false) {

  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  uuid_ = boost::uuids::to_string(uuid);
  recv_head_node_ = std::make_shared<MsgNode>(HEAD_TOTAL_LENGTH);
}

Session::~Session() {
  try {
    std::cout << "Session destruct";
    close();
  } catch (std::exception &e) {
    std::cout << "Session desturct error occured ,error is " << e.what()
              << std::endl;
  }
}

asio::ip::tcp::socket &Session::get_socket() { return socket_; }
std::string &Session::get_uuid() { return uuid_; }

void Session::send(char const *_msg, short _max_len, short _msg_id) {
  std::unique_lock<std::mutex> gaurd(mutex_);
  int send_que_size = send_que_.size();
  if (send_que_size > MAX_SENDQUE_SIZE) {
    std::cout << "session:" << uuid_ << "send que exceed max length"
              << std::endl;
    return;
  }
  send_que_.push(std::make_shared<SendNode>(_msg, _max_len, _msg_id));
  //
  if (send_que_size > 0) {
    return;
  }
  auto msg_node = send_que_.front();
  gaurd.unlock();
  asio::async_write(socket_, asio::buffer(msg_node->data, msg_node->total_len),
                    std::bind(&Session::write_callback_, this,
                              std::placeholders::_1, shared_from_this()));
}

void Session::send(std::string const &_msg, short _msg_id) {
  send(_msg.c_str(), _msg.length(), _msg_id);
}

void Session::start() {
  auto shared_this = shared_from_this();

  asio::co_spawn(io_context_, [shared_this, this]() -> asio::awaitable<void> {
    try {
    std::cout << "start handle msg.." << std::endl;
      while (!b_close_) {
        recv_head_node_->clear();
        std::size_t n = co_await asio::async_read(
            socket_, asio::buffer(recv_head_node_->data, HEAD_TOTAL_LENGTH),
            asio::use_awaitable);

        if (n == 0) {
          std::cout << "receive peer closed" << std::endl;
          close();
          server_->remove_session(uuid_);
          co_return;
        }

        // 读出消息ID
        short msg_id = 0;
        memcpy(&msg_id, recv_head_node_->data, HEAD_ID_LENGTH);
        msg_id = asio::detail::socket_ops::network_to_host_short(msg_id);
        std::cout << "msg id is " << msg_id << std::endl;
        if (msg_id > MAX_MSG_LENGTH) {
          std::cout << "Invalid msg id: " << msg_id << std::endl;
          close();
          server_->remove_session(uuid_);
          co_return;
        }
        // 读出消息长度
        short msg_len;
        memcpy(&msg_len, recv_head_node_->data + HEAD_ID_LENGTH,
               HEAD_DATA_LENGTH);
        msg_len = asio::detail::socket_ops::network_to_host_short(msg_len);
        std::cout << "msg len is " << msg_len << std::endl;
        if (msg_len > MAX_MSG_LENGTH) {
          std::cout << "Invalid msg len : " << msg_len << std::endl;
          close();
          server_->remove_session(uuid_);
          co_return;
        }
        recv_msg_node_ = std::make_shared<RecvNode>(msg_len, msg_id);

        // 读出包体消息
        n = co_await asio::async_read(
            socket_,
            asio::buffer(recv_msg_node_->data, recv_msg_node_->total_len),
            asio::use_awaitable);

        if (n == 0) {
          std::cout << "receive peer closed" << std::endl;
          close();
          server_->remove_session(uuid_);
          co_return;
        }

        recv_msg_node_->data[recv_msg_node_->total_len] = '\0';
        // 将消息投递给逻辑线程
        LogicSystem::get_instance().post_msg_to_que(std::make_shared<LogicNode>(shared_this, recv_msg_node_));
      }
    } catch (std::exception &e) {
      std::cout << "Exception ocurred, error is " << e.what() << std::endl;
      close();
      server_->remove_session(uuid_);
    }
  }, asio::detached);
}

void Session::close() {
  socket_.close();
  b_close_ = true;
}

void Session::write_callback_(asio::error_code _ec,
                              std::shared_ptr<Session> shared_self) {
  try {
    if (!_ec) {
      std::unique_lock<std::mutex> gaurd(mutex_);
      send_que_.pop();
      if (!send_que_.empty()) {
        auto msg_node = send_que_.front();
        gaurd.unlock();
        asio::async_write(socket_,
                          asio::buffer(msg_node->data, msg_node->total_len),
                          std::bind(&Session::write_callback_, this,
                                    std::placeholders::_1, shared_from_this()));
      }
    } else {

      std::cout << "session write error occured, error is " << _ec.message()
                << std::endl;
      close();
      server_->remove_session(uuid_);
    }
  } catch (std::exception &e) {
    std::cout << "session write exception occured, error is " << e.what()
              << std::endl;
    close();
    server_->remove_session(uuid_);
  }
}
