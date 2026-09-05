
#include <asio/io_context.hpp>
#include <asio/signal_set.hpp>
#include <condition_variable>
#include <csignal>
#include <exception>
#include <iostream>
#include <mutex>
#include <thread>

#include "../include/server.hpp"
#include "../include/asio_thread_pool.hpp"

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


void cstyle_quit_server(AsioThreadPool & pool)
{

  try {
    asio::io_context ioc;
    std::thread net_work_thread([&pool]() {
      CServer s(pool.get_io_context(), 10086);
    });
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    while(!g_stop_server)
    {
        std::unique_lock<std::mutex> ulock(g_quit_mutex);
        g_quit_cond.wait(ulock);
    }

    ioc.stop(); // 关闭ioc服务
    pool.stop();
    net_work_thread.join();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}



void asio_quit_server(AsioThreadPool & pool){
  
    try {
    auto ioc = asio::io_context{}; 
    // 注册信号事件
    asio::signal_set signal(ioc, SIGINT, SIGTERM);
    signal.async_wait([&ioc, &pool](auto, auto){
        // signal注册的信号触发时，停止ioc服务，
        ioc.stop();
        pool.stop();
       // std::unique_lock<std::mutex> ulock(g_quit_mutex);
        //g_stop_server = true;
        //g_quit_cond.notify_one();
        std::cout << "stop finished\n";
    });

    // 使用池中的io_context, 池中的io_context.run运行在多个线程中，可以多线程派发事件，再由strand统一串行化调度回调
    CServer s(pool.get_io_context(), 10086);
    ioc.run();

    {

        //std::unique_lock<std::mutex> ulock(g_quit_mutex);
        //while(!g_stop_server){
          //g_quit_cond.wait(ulock);
        //}
    }

    // 这里不用io_context.run阻塞了，使用条件变量阻塞，
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
    
}

int main(int argc, char *argv[]) {
    auto pool = AsioThreadPool::get_instance()->get_instance();
    asio_quit_server(*pool);
  return 0;
}