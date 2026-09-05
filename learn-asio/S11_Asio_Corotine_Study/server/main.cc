
#include <asio/use_awaitable.hpp>
#include <cstddef>
#include <exception>
#include <iostream>

#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp> // 包含启动协程函数
#include <asio/detached.hpp> // 可以分离协程，让一个线程内运行多个协程
#include <asio/io_context.hpp> // 使用io_context作为协程调度器
#include <asio/ip/tcp.hpp>
#include <asio/signal_set.hpp> // 捕捉信号优雅退出
#include <asio/this_coro.hpp>
#include <asio/write.hpp>

asio::awaitable<void> echo(asio::ip::tcp::socket socket) {
  try {
    char data[1024];
    while (true) {
      // 使用协程转为同步方式, 等待读取到数据时从此恢复
      std::size_t n = co_await socket.async_read_some(asio::buffer(data),
                                                      asio::use_awaitable);
      std::cout << "read data " << data << " size: " << n << std::endl;
      // 使用协程方式转为同步方式，等待写完数据后从此恢复执行，继续下轮循环
      co_await async_write(socket, asio::buffer(data, n), asio::use_awaitable);
      std::cout << "write to remote endpoint: " << std::string(data, n) << std::endl;
    }
  } catch (std::exception const &e) {
    std::cout << "Echo Exception " << e.what() << std::endl;
  }
}

//  返回值使用asio::awaitable包装为一个Awaitable对象，以便协程使用
asio::awaitable<void> listen() {
  // 获取协程的调度器
  auto excutor = co_await asio::this_coro::executor;
  asio::ip::tcp::acceptor acceptor(
      excutor, asio::ip::tcp::endpoint{asio::ip::tcp::v4(), 10086});

  // 使用协程，可以将异步回调变为同步调用代码
  while (true) {
    // 协程暂停点，
    // 可以在此处释放协程控制权给CPU，让主线程继续执行。
    // 当acceptor监听到连接时，协程可以从这里恢复继续往下执行。

    // asio::use_awaiable告诉asio使用协程方式调度异步函数async_accept,
    // 让该函数成为可等待的协程函数
    asio::ip::tcp::socket socket =
        co_await acceptor.async_accept(asio::use_awaitable);
    std::cout << "listened accept request from "
              << socket.remote_endpoint().address().to_string() << std::endl;
    // 调度另一个协程，执行echo操作回传消息
    asio::co_spawn(excutor, echo(std::move(socket)), asio::detached);
  }
}

int main(int argc, char *argv[]) {
  try {
    asio::io_context ioc(1);
    asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&ioc](auto, auto) { ioc.stop(); });

    // 启动协程
    asio::co_spawn(
        ioc,      // 调度器
        listen(), // Awaitable对象，
        asio::detached); // 让协程独立运行，主线程阻塞等待协程执行完毕
    ioc.run();
  } catch (std::exception const &e) {
    std::cout << "Exception " << e.what() << std::endl;
  }
}