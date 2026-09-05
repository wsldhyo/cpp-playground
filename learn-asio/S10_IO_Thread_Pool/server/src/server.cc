#include <asio/io_context.hpp>
#include <boost/uuid/random_generator.hpp>
#include <iostream>

#include "../include/server.hpp"

CServer::CServer(asio::io_context &io_context, short port)
    : port_(port), io_context_(io_context),
                       acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
  std::cout << "Server start success, listen on port : " << port_ << std::endl;
  start_accept_();
}

void CServer::accept_callback_(std::shared_ptr<CSession> new_session,
                               const asio::error_code &error) {
  if (!error) {
    new_session->start();
    sessions_.insert(make_pair(new_session->get_uuid(), new_session));
  } else {
    std::cout << "session accept failed, error is " << error.message()
              << std::endl;
  }

  start_accept_();
}

void CServer::start_accept_() {
  std::shared_ptr<CSession> new_session =
      std::make_shared<CSession>(io_context_, this);
  // acceptor_用的io_context是主线程的，与CSession中的不一样
  acceptor_.async_accept(new_session->get_socket(),
                         std::bind(&CServer::accept_callback_, this,
                                   new_session, std::placeholders::_1));
}

void CServer::remove_session(std::string uuid) { sessions_.erase(uuid); }