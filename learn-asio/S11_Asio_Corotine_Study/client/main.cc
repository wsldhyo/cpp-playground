#include <asio.hpp>
#include <asio/error.hpp>
#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/system_error.hpp>
#include <asio/write.hpp>
#include <cstdio>
#include <iostream>
int const MAX_LENGTH{1024};
int main(int argc, char *argv[]) {
  try {
    asio::io_context ioc;
    asio::ip::tcp::endpoint remote_ep(asio::ip::make_address("127.0.0.1"),
                                      10086);
    asio::ip::tcp::socket socket(ioc);
    asio::error_code error = asio::error::host_not_found;
    socket.connect(remote_ep, error);
    if (error) {
      std::cout << "connect failed code is " << error.value() << "error msg is "
                << error.message() << std::endl;
      return 1;
    }
   // while (true) {

      std::cout << "Enter Message:";
      char request[MAX_LENGTH];
      std::cin.getline(request, MAX_LENGTH);
      std::cout << "request msg is " << request << std::endl;
      auto request_len = strlen(request);
      asio::write(socket, asio::buffer(request, request_len));
      char reply_buf[MAX_LENGTH];
      auto reply_len = asio::read(socket, asio::buffer(reply_buf, request_len));
      std::cout << "reply is " << std::string(reply_buf, reply_len)
                << std::endl;
      getchar();
      getchar();
//    }
  } catch (std::exception const &e) {
    std::cout << "Exception is " << e.what() <<std::endl;
  }
  return 0;
}