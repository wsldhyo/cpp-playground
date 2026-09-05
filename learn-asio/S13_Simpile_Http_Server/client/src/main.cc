#include "../include/client.hpp"
#include <asio/io_context.hpp>
#include <cstdio>
#include <exception>
#include <iostream>

int main(int argc, char *argv[]) {
  try {
    asio::io_context ioc;
    // 8080端口用于WWW服务
    // 路由/表示请求根目录，服务器会返回index.html， 
    Client c(ioc, "127.0.0.1:8080", "/");
    ioc.run();
    getchar();
  } catch (std::exception &e) {
    std::cout << "Client exception: " << e.what() << std::endl;
  }
  return 0;
}