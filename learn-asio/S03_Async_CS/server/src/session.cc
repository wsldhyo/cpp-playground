#include "../include/server.hpp"
#include <asio/error_code.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/system_error.hpp>
#include <asio/write.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iostream>

SessionErr::SessionErr(asio::io_context &ioc) : sock_(ioc) {}

asio::ip::tcp::socket &SessionErr::socket() { return sock_; }

void SessionErr::start() {
  memset(data_, 0, MAX_LENGTH);
  // 读到多少返回多少
  // 如果使用read，会阻塞直到读取到MAX_LENGTH长度的数据才返回，服务器一般不用，影响效率
  sock_.async_read_some(asio::buffer(data_, MAX_LENGTH),
                        std::bind(&SessionErr::handle_read_, this,
                                  std::placeholders::_1,
                                  std::placeholders::_2));
}
// 当客户端写入数据时，服务器缓冲区收到客户端的数据后，
// 将触发读事件，随后该回调函数被调用，将TCP缓冲区的数据写入应用层缓冲区
void SessionErr::handle_read_(asio::error_code const &ec,
                              std::size_t bytes_tranferred) {
  if (!ec) {
    std::cout << "server receive data is " << data_ << std::endl;
    // 回传数据给对方，向TCP缓冲区写入数据，当Asio完成写入后，将产生一个写事件，触发handle_write_回调
    asio::async_write(
        sock_, asio::buffer(data_, bytes_tranferred),
        std::bind(&SessionErr::handle_write_, this, std::placeholders::_1));

  } else {
    std::cout << "connection is closed by peer" << std::endl;
    delete this;
  }
}

// 当应用层向TCP缓冲区写入数据时，将产生一个写事件
// 随后该回调函数被调用，调用async_read_some，准备读取下一个客户端数据
void SessionErr::handle_write_(asio::error_code const &ec) {

  if (!ec) {
    memset(data_, 0, MAX_LENGTH);
    sock_.async_read_some(asio::buffer(data_, MAX_LENGTH),
                          std::bind(&SessionErr::handle_read_, this,
                                    std::placeholders::_1,
                                    std::placeholders::_2));

  } else {
    std::cout << "write error" << std::endl;
    delete this;
  }
}

CSession::CSession(asio::io_context &ioc, CServer *server)
    : sock_(ioc), server_(server) {

  boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
  uuid_ = boost::uuids::to_string(a_uuid);
}

std::string &CSession::get_uuid() { return uuid_; }

asio::ip::tcp::socket &CSession::socket() { return sock_; }

void CSession::start() {
  memset(data_, 0, MAX_LENGTH);
  // 不能使用shared_ptr<CSession>(this)、make_shared等方式创建智能指针，多次调用start()会导致多个智能指针管理同一快内存，引用计数不同步
  // 几不能使用相同的裸指针创建多个不同的智能指针
  // shared_from_this保证了生成的多个智能指针的引用计数同步
  auto sp_self = shared_from_this();
  sock_.async_read_some(asio::buffer(data_, MAX_LENGTH),
                        std::bind(&CSession::handle_read_, this,
                                  std::placeholders::_1, std::placeholders::_2,
                                  sp_self));
}

void CSession::handle_read_(asio::error_code const &ec,
                            std::size_t bytes_tranferred,
                            std::shared_ptr<CSession> sp_self) {
  if (!ec) {
    std::cout << "server receive data is " << data_ << std::endl;
    asio::async_write(sock_, asio::buffer(data_, bytes_tranferred),
                      std::bind(&CSession::handle_write_, this,
                                std::placeholders::_1, sp_self));

  } else {
    std::cout << "connection is closed by peer" << std::endl;
    // delete this;
    server_->remove_session(uuid_);
  }
}
void CSession::handle_write_(asio::error_code const &ec,
                             std::shared_ptr<CSession> sp_self) {

  if (!ec) {
    memset(data_, 0, MAX_LENGTH);
    sock_.async_read_some(asio::buffer(data_, MAX_LENGTH),
                          std::bind(&CSession::handle_read_, this,
                                    std::placeholders::_1,
                                    std::placeholders::_2, sp_self));

  } else {
    std::cout << "write error" << std::endl;
    // delete this;
    server_->remove_session(uuid_);
  }
}


CSession2::CSession2(asio::io_context &ioc, CServer2 *server)
    : sock_(ioc), server_(server) {

  boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
  uuid_ = boost::uuids::to_string(a_uuid);
}

std::string &CSession2::get_uuid() { return uuid_; }

asio::ip::tcp::socket &CSession2::socket() { return sock_; }

void CSession2::start() {
  memset(data_, 0, MAX_LENGTH);
  // 不能使用shared_ptr<CSession>(this)、make_shared等方式创建智能指针，多次调用start()会导致多个智能指针管理同一快内存，引用计数不同步
  // 几不能使用相同的裸指针创建多个不同的智能指针
  // shared_from_this保证了生成的多个智能指针的引用计数同步
  auto sp_self = shared_from_this();
  sock_.async_read_some(asio::buffer(data_, MAX_LENGTH),
                        std::bind(&CSession2::handle_read_, this,
                                  std::placeholders::_1, std::placeholders::_2,
                                  sp_self));
}


void CSession2::send(char *msg, int max_length) {
  // 不能保证发送的接口和回调函数的接口在一个线程，所以要增加一个锁保证发送队列安全性
  std::lock_guard<std::mutex> lock(send_lock_);
  bool pending =
      false; // true表示当前发送队列里有数据，即send上次调用的传递数据还未发送完毕
  if (send_que_.size() > 0) {
    pending = true;
  }
  send_que_.push(std::make_shared<MsgNode>(msg, max_length));
  // pendind为false，则说明有数据需要发送，不能直接return，需要提交异步写操作
  if (pending) {
    return;
  }
  asio::async_write(sock_, asio::buffer(msg, max_length),
                    std::bind(&CSession2::handle_write_, this,
                              std::placeholders::_1, shared_from_this()));
}

void CSession2::handle_write_(asio::error_code const &ec,
                              std::shared_ptr<CSession2> sp_self) {
  if (!ec) {
    std::lock_guard<std::mutex> lock(send_lock_);
    send_que_.pop();   // ec不为0，前次调用发送完队首元素，所以去掉队首元素
    if (!send_que_.empty()) {
      // 队列不空，则继续发送
      auto &msgnode = send_que_.front();
      asio::async_write(
          sock_, asio::buffer(msgnode->data, msgnode->max_len),
          std::bind(&CSession2::handle_write_,  this, std::placeholders::_1,
                    sp_self));
    }
  } else {  
    std::cout << "handle write failed, error is " << ec.message()<< std::endl;
    server_->remove_session(uuid_);
  }
}

void CSession2::handle_read_(const asio::error_code& ec, size_t  bytes_transferred, std::shared_ptr<CSession2> _self_shared){
    if (!ec) {
        std::cout << "read data is " << data_ << std::endl;
        //发送数据
        send(data_, bytes_transferred);
        memset(data_, 0, MAX_LENGTH);
        sock_.async_read_some(asio::buffer(data_, MAX_LENGTH), std::bind(&CSession2::handle_read_, this, 
            std::placeholders::_1, std::placeholders::_2, _self_shared));
    }
    else {
        std::cout << "handle read failed, error is " << ec.message() << std::endl;
        server_->remove_session(uuid_);
    }
}