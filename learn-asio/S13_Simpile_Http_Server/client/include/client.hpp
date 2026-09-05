#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>
using asio::ip::tcp;

class Client {
public:

/**
 * @brief Construct a new Client:: Client object
 * 
 * @param _ioc 
 * @param _server_addr 服务端地址，可能是域名，需要借助resolver解析为ip
 * @param _path 请求的路由地址
 */
  Client(asio::io_context& _ioc, std::string const& _server_addr,
         std::string const& _path);

private:
    void resolve_callback_(asio::error_code _ec, tcp::resolver::results_type _endpoint);
    void connect_callback_(asio::error_code _ec);
    void write_callback_(asio::error_code _ec);
    void read_status_line_callback_(asio::error_code _ec);
    void read_headers_callback_(asio::error_code _ec);
    void read_content_callback_(asio::error_code _ec);
    tcp::resolver resolver_;
    tcp::socket socket_;
    asio::streambuf request_;
    asio::streambuf response_;
};

#endif