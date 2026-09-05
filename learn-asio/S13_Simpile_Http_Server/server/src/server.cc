#include "../include/server.hpp"
#include "../include/io_context_pool.hpp"
#include "../include/session.hpp"
#include <asio/ip/tcp.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <utility>
Server::Server(asio::io_context &_io_context, short _port)
    : io_context_(_io_context), port_(_port),
      acceptor_(io_context_,
                asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port_)) {

  start_accept_();
}

Server::~Server() {}

void Server::accept_callback_(std::shared_ptr<Session> _session,
                              asio::error_code _ec) {
  if (!_ec) {
    _session->start();
    std::lock_guard<std::mutex> gaurd(mutex_);
    sessions_.insert(std::make_pair(_session->get_uuid(), _session));
  } else {
    std::cout << "session accept failed, error is " << _ec.message()
              << std::endl;
  }
  start_accept_();
}

void Server::start_accept_() {
  auto &io_context = IOContextPool::get_instance().get_io_context();
  std::shared_ptr<Session> new_session =
      std::make_shared<Session>(io_context, this);
  acceptor_.async_accept(new_session->get_socket(),
                         std::bind(&Server::accept_callback_, this, new_session,
                                   std::placeholders::_1));
}

void Server::remove_session(std::string &_uuid) {
  std::lock_guard<std::mutex> gaurd(mutex_);
  sessions_.erase(_uuid);
}