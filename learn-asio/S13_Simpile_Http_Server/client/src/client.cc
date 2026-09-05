#include "../include/client.hpp"
#include <asio/completion_condition.hpp>
#include <asio/connect.hpp>
#include <asio/error_code.hpp>
#include <asio/placeholders.hpp>
#include <asio/read.hpp>
#include <asio/read_until.hpp>
#include <asio/write.hpp>
#include <cstddef>
#include <iostream>
#include <string>

Client::Client(asio::io_context &_ioc, std::string const &_server_addr,
               std::string const &_path)
    : resolver_(_ioc), socket_(_ioc) {

  // 构造HTTP请求报文
  std::ostream os(&request_);
  os << "GET " << _path << " HTTP/1.0\r\n";
  os << "Host " << _server_addr << "\r\n";
  os << "Accept:*/*\r\n";           // */*表示服务器返回的全部接受
  os << "Connection:close\r\n\r\n"; //   tcp短连接，
                                    //   完成一次通信后，C/S任意一端可以断开连接

  // 拆除IP和端口
  std::size_t colon_pos = _server_addr.find(":");
  std::string ip = _server_addr.substr(0, colon_pos);
  std::string port = _server_addr.substr(colon_pos + 1);
  resolver_.async_resolve(ip, port,
                          std::bind(&Client::resolve_callback_, this,
                                    asio::placeholders::error,
                                    asio::placeholders::results));
}

void Client::resolve_callback_(asio::error_code _ec,
                               tcp::resolver::results_type _endpoint) {
  if (!_ec) {
    asio::async_connect(
        socket_, _endpoint,
        std::bind(&Client::connect_callback_, this, asio::placeholders::error));
  } else {
    std::cout << "resolve error: " << _ec.message() << std::endl;
  }
}

void Client::connect_callback_(asio::error_code _ec) {
  if (!_ec) {
    asio::async_write(
        socket_, request_,
        std::bind(&Client::write_callback_, this, asio::placeholders::error));
  } else {
    std::cout << "connect error: " << _ec.message() << std::endl;
  }
}

void Client::write_callback_(asio::error_code _ec) {
  if (!_ec) {
    asio::async_read_until(socket_, request_, "\r\n",
                           std::bind(&Client::read_status_line_callback_, this,
                                     asio::placeholders::error));
  } else {
    std::cout << "write error: " << _ec.message() << std::endl;
  }
}

void Client::read_status_line_callback_(asio::error_code _ec) {
  if (!_ec) {
    std::istream is(&response_);

    // 读取HTTP报文头部信息，包括HTTP版本、状态吗和状态消息
    std::string http_version;
    is >> http_version;
    unsigned int status_code;
    is >> status_code;
    std::string status_msg;
    is >> status_msg;
    // 取出下一行消息，必须有下一行
    std::getline(is, status_msg);
    if (!is || http_version.substr(0, 5) != "HTTP/") {
      std::cout << "invalid response\n";
      return;
    }

    if (status_code != 200) {
      // 非成功状态
      std::cout << "request failed, status code: " << status_code << std::endl;
      return;
    }
    // HTTP Request报文中头部和消息体间有一个空行，加上头部最后一行的空行，
    // 需要读取两个换行，才能读取完头部并为直接读取消息做好准则
    asio::async_read_until(socket_, response_, "\r\n\r\n",
                           std::bind(&Client::read_headers_callback_, this,
                                     asio::placeholders::error));
  } else {
    std::cout << "read status line error: " << _ec.message() << std::endl;
  }
}
void Client::read_headers_callback_(asio::error_code _ec) {

  if (!_ec) {
    std::istream is(&response_);
    std::string header;
    // 逐行读取，并打印头部信息
    while (std::getline(is, header) && header != "\r") {
      std::cout << header << std::endl;
    }
    std::cout << std::endl;
    if (response_.size() > 0) {
      std::cout << &response_;
    }

    asio::async_read(socket_, response_,
                     asio::transfer_at_least(1), // 至少读取一个字节
                     std::bind(&Client::read_content_callback_, this,
                               asio::placeholders::error));
  } else {
    std::cout << "read headers line error: " << _ec.message() << std::endl;
  }
}

void Client::read_content_callback_(asio::error_code _ec) {

  if (!_ec) {
    // 打印读取到的消息
    std::cout << &response_;

    // 因为是短连接，立即挂起读内容回调，以读取服务器关闭连接消息。然后回调此函数进入else语句结束本次收发
    asio::async_read(socket_, response_, asio::transfer_at_least(1),
                     std::bind(&Client::read_content_callback_, this,
                               asio::placeholders::error));
  }
  else {
    std::cout << "read content error: " << _ec.message() << std::endl;
  }
}