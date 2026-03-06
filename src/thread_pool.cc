#include "thread_pool.hpp"
#include <iostream>
#include <mutex>
#include <thread>

thread_local bool LThreadPool::is_worker_thread_ = false;
thread_local std::queue<LThreadPool::task_t> LThreadPool::local_tasks_;
LThreadPool::LThreadPool(std::size_t thread_num) : stop_(false) {
  for (std::size_t i = 0; i < thread_num; ++i) {
    // 构造任务线程，尝试从池中取出任务并执行
    pool_.emplace_back([this]() {
      is_worker_thread_ = true; // 标识自己是工作线程
      auto id = std::hash<std::thread::id>{}(std::this_thread::get_id());
      while (true) {
        {
          task_t task = nullptr;
          if (!local_tasks_.empty()) {
            task = local_tasks_.front();
            local_tasks_.pop();
          } else {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });
            if (stop_ && tasks_.empty()) {
              // 本地队列还有任务
              if (!local_tasks_.empty()) {
                continue;
              }
              // 因停止而唤醒，执行完所有任务后才结束， 防止丢任务
              break;
            }
            // 一定是有任务到了,无需判断tasks空
            task = tasks_.front();
            tasks_.pop();
          }
          task();
        }
      }
    });
  }
}

LThreadPool::~LThreadPool() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stop_ =
        true; // 任务线程中的cond_.wait会立即返回，这样线程可以尽快执行任务并退出
  }

  cond_.notify_all(); // 要通知所有等待线程
  for (auto &thread : pool_) {
    if (thread.joinable()) {
      thread.join();
    }
  }
  std::cout << "thread pool finished\n";
}
