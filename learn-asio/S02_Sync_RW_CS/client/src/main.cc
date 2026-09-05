
#include <asio.hpp>
#include <asio/error.hpp>
#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/system_error.hpp>
#include <cstddef>
#include <exception>
#include <iostream>
using asio::ip::tcp;

std::size_t constexpr MAX_MSG_LEN{1024};
int main(int argc, char *argv[]) {
  try {
    asio::io_context ioc;
    tcp::endpoint remote_ep(asio::ip::make_address("127.0.0.1"), 10086);
    asio::error_code ec;
    tcp::socket sock(ioc);
    sock.connect(remote_ep, ec);
    if (ec.value() != 0) {
      std::cout << "connect failed error code = " << ec.value()
                << " Message: " << ec.message() << std::endl;
      return ec.value();
    }
    while (true) {

      std::cout << "Enter message:";
      char buf[MAX_MSG_LEN];
      std::cin.getline(buf, MAX_MSG_LEN);
      auto actual_send_len = strlen(buf);
      asio::write(sock, asio::buffer(buf, actual_send_len));
      char reply_buf[MAX_MSG_LEN];
      auto reply_len{
          asio::read(sock, asio::buffer(reply_buf, actual_send_len))};
      std::cout << "Reply: " << reply_buf << std::endl;
    }

  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  return 0;
}
