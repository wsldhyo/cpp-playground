#include "../include/asio_thread_pool.hpp"

AsioThreadPool::AsioThreadPool(
    std::size_t _thread_num /* = std::thread::hardware_concurrency()*/)
    : work_(std::make_unique<Work_t>(asio::make_work_guard(io_context_))) {
  // 初始化列表先初始化work_，以让线程启动时，io_context_.run可以阻塞
  for (std::size_t i = 0; i < _thread_num; ++i) {
    threads_.emplace_back([this]() { io_context_.run(); });
  }
}

AsioThreadPool::~AsioThreadPool() { stop(); }

asio::io_context &AsioThreadPool::get_io_context() { return io_context_; }

void AsioThreadPool::stop() {
  // 让work_指向的Work析构，从而使得Work与io_context解绑，
  // 让io_context.run结束阻塞后退出
  work_->reset();
  io_context_.stop();
  for (auto &thread : threads_) {
    thread.join();
  }
}
