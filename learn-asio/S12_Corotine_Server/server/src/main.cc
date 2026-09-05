#include "../include/io_context_pool.hpp"
#include "../include/server.hpp"
#include <asio/signal_set.hpp>
#include <csignal>
#include <exception>
#include <iostream>

int main(int argc, char *argv[]) {
  try {
    auto &pool = IOContextPool::get_instance();
    asio::io_context ioc; 
    asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&ioc, &pool](auto, auto){
        ioc.stop();
        pool.stop();
    });
    Server server(ioc, 10086);
    ioc.run();
  } catch (std::exception &e) {
    std::cout << "Error occur, error is " << e.what() << std::endl;
  }
}