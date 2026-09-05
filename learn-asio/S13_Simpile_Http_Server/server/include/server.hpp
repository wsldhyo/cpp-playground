#ifndef SERVER_HPP
#define SERVER_HPP

#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <map>
#include <memory>
#include <mutex>

class Session;

class Server {
public:
    Server(asio::io_context& _io_context, short _port);
    ~Server();
    void remove_session(std::string& _uuid);
private:
    void accept_callback_(std::shared_ptr<Session> _session, asio::error_code _ec);
    void start_accept_();

    asio::io_context& io_context_;
    short port_;
    asio::ip::tcp::acceptor acceptor_;
    std::map<std::string, std::shared_ptr<Session>> sessions_;
    std::mutex mutex_;
};
#endif