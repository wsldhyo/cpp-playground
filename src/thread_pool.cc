#include "thread_pool.hpp"
#include <iostream>
#include <mutex>
#include <thread>
ThreadPool::ThreadPool(std::size_t thread_num) : stop_(false) {
  for (std::size_t i = 0; i < thread_num; ++i) {
    // 构造任务线程，尝试从池中取出任务并执行
    pool_.emplace_back([this]() {
      auto id = std::hash<std::thread::id>{}(std::this_thread::get_id());
      while (true) {
        {
          task_t task = nullptr;
          {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });
            if (stop_ &&
                tasks_
                    .empty()) { // 如果停止了，并且没有任务才结束线程，避免丢任务
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

ThreadPool::~ThreadPool() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stop_ = true; // 任务线程中的cond_.wait会立即返回，这样线程可以尽快执行任务并退出
  }

  cond_.notify_all(); // 要通知所有等待线程
  for (auto &thread : pool_) {
    if (thread.joinable()) {
      thread.join();
    }
  }
  std::cout << "thread pool finished\n";
}

void ThreadPool::submit(task_t task) {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    if (stop_) { // 线程停止后，不应该再提交任务
      std::cout << "Thread pool has stopped!submit task failed!\n";
      return;
    }
    tasks_.push(task);
  }
  cond_.notify_one();
}
