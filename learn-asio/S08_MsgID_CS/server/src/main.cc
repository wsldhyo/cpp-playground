
#include <asio/io_context.hpp>
#include <asio/signal_set.hpp>
#include <condition_variable>
#include <csignal>
#include <exception>
#include <iostream>
#include <mutex>
#include <thread>

#include "../include/server.hpp"

bool g_stop_server{false};
std::mutex g_quit_mutex;
std::condition_variable g_quit_cond;

void signal_handler(int sig) {
  if (sig == SIGINT || sig == SIGTERM) {
    std::unique_lock<std::mutex> ulock(g_quit_mutex);
    g_stop_server = true;
    g_quit_cond.notify_one();
  }
}


void cstyle_quit_server()
{

  try {
    asio::io_context ioc;
    // ServerErr s(ioc, 10086);

    std::thread net_work_thread([&ioc]() {
      CServer s(ioc, 10086);
      ioc.run(); // 启动事件循环
    });
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    while(!g_stop_server)
    {
        std::unique_lock<std::mutex> ulock(g_quit_mutex);
        g_quit_cond.wait(ulock);
    }

    ioc.stop(); // 关闭ioc服务
    net_work_thread.join();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}




void asio_quit_server(){
    try {
    asio::io_context ioc;
    // 注册信号事件
    asio::signal_set signal(ioc, SIGINT, SIGTERM);
    signal.async_wait([&ioc](auto, auto){
        // signal注册的信号触发时，停止ioc服务，
        ioc.stop();
    });

    CServer s(ioc, 10086);
    ioc.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
    
}

int main(int argc, char *argv[]) {
    asio_quit_server();
  return 0;
}