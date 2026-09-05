#include "../include/server.hpp"
#include <boost/uuid/random_generator.hpp>
#include <iostream>
ServerErr::ServerErr(asio::io_context &ioc, short port_num)
    : ioc_(ioc),
      acceptor_(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port_num)) {

  std::cout << "ServerErr start success on port:" << port_num << std::endl;
  start_accept();
}

void ServerErr::start_accept() {
  auto new_session = new SessionErr(ioc_);
  // std::bind按值传递智能指针，引用计数+1。
  // 该函数结束，new_session变量生命周期结束，
  // 但智能指针的生命周期与std::bind返回的函数对象一致
  acceptor_.async_accept(new_session->socket(),
                         std::bind(&ServerErr::handle_accept, this, new_session,
                                   std::placeholders::_1));
}
void ServerErr::handle_accept(SessionErr *new_session,
                              asio::error_code const &ec) {
  if (!ec) {
    std::cout << "connect client from: "
              << new_session->socket().remote_endpoint().address().to_v4()
              << "!\n";
    new_session->start();
  } else {
    delete new_session;
  }

  // 接受新连接
  start_accept();
}

CServer::CServer(asio::io_context &ioc, short port_num)
    : ioc_(ioc),
      acceptor_(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port_num)) {

  std::cout << "CServerstart success on port:" << port_num << std::endl;
  start_accept();
}

void CServer::start_accept() {
  auto new_session = std::make_shared<CSession>(ioc_, this);
  // std::bind按值传递智能指针，引用计数+1。
  // 该函数结束，new_session变量生命周期结束，n
  // 但智能指针的生命周期与std::bind返回的函数对象一致
  acceptor_.async_accept(new_session->socket(),
                         std::bind(&CServer::handle_accept, this, new_session,
                                   std::placeholders::_1));
}
void CServer::handle_accept(std::shared_ptr<CSession> new_session,
                            asio::error_code const &ec) {
  if (!ec) {
    std::cout << "connect client from: "
              << new_session->socket().remote_endpoint().address().to_v4()
              << "!\n";

    new_session->start();
    sessions_.insert(std::make_pair(new_session->get_uuid(), new_session));
  }
  else {
  
  }

  // 接受新连接
  start_accept();
}

void CServer::remove_session(std::string& uuid)
{
    sessions_.erase(uuid);
}
CServer2::CServer2(asio::io_context &ioc, short port_num)
    : ioc_(ioc),
      acceptor_(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port_num)) {

  std::cout << "CServerstart success on port:" << port_num << std::endl;
  start_accept();
}

void CServer2::start_accept() {
  auto new_session = std::make_shared<CSession2>(ioc_, this);
  // std::bind按值传递智能指针，引用计数+1。
  // 该函数结束，new_session变量生命周期结束，n
  // 但智能指针的生命周期与std::bind返回的函数对象一致
  acceptor_.async_accept(new_session->socket(),
                         std::bind(&CServer2::handle_accept, this, new_session,
                                   std::placeholders::_1));
}
void CServer2::handle_accept(std::shared_ptr<CSession2> new_session,
                            asio::error_code const &ec) {
  if (!ec) {
    std::cout << "connect client from: "
              << new_session->socket().remote_endpoint().address().to_v4()
              << "!\n";

    new_session->start();
    sessions_.insert(std::make_pair(new_session->get_uuid(), new_session));
  }
  else {
  
  }

  // 接受新连接
  start_accept();
}

void CServer2::remove_session(std::string& uuid)
{
    sessions_.erase(uuid);
}