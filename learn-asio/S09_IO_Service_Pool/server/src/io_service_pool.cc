#include <cstddef>
#include <iostream>
#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include "../include/io_service_pool.hpp"

IOServicePool::IOServicePool(
    std::size_t size /*=std::thread::hareward_concurrency()*/)
    : io_contexts_(size), Works_(size), next_io_context_index_(0)

{
  // 先让work和io_context绑定，以便让io_context.run()阻塞
  for (std::size_t i = 0; i < size; ++i) {
    Works_[i] = std::move(
        std::make_unique<Work_t>(asio::make_work_guard(io_contexts_[i])));
  }

  // 创建多个线程，每个线程启动一个io_context
  for (std::size_t i = 0; i < size; ++i) {
    threads_.emplace_back([this, i]() { io_contexts_[i].run(); });
  }
}

IOServicePool::~IOServicePool(){
    std::cout << "~IOServicePool" << std::endl;
}

asio::io_context& IOServicePool::get_io_context(){
    // 每次取出一个与上次不同的io_context
    auto& io_context{io_contexts_[next_io_context_index_++]};
    if (next_io_context_index_ == io_contexts_.size()) {
        next_io_context_index_ = 0;
    }
    return io_context;  
}

void IOServicePool::stop(){
    for (auto& work : Works_) {
        // 重置work指针，触发work析构操作
        // work析构时会解除io_context所绑定的事件，从而使得asio能够解除run的阻塞
        work->reset();
    }

    // 等待所有线程的io_context结束阻塞
    for (auto& t : threads_) {
        if (t.joinable()) {
            t.join();
        }
    }

}