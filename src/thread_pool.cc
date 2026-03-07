#include "thread_pool.hpp"
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <thread>
#include <algorithm>

thread_local bool LThreadPool::is_worker_thread_{false};
thread_local std::unique_ptr<LThreadPool::LocalQueue> LThreadPool::local_tasks_{
    nullptr};
LThreadPool::LThreadPool(std::size_t thread_num) : stop_(false) {
  tasks_ptrs_.resize(thread_num);
  for (std::size_t i = 0; i < thread_num; ++i) {
    // 构造任务线程，尝试从池中取出任务并执行
    pool_.emplace_back([this, i]() {
      is_worker_thread_ = true; // 标识自己是工作线程
      local_tasks_ = std::make_unique<LocalQueue>();
      // 注册本地队列
      {
        std::lock_guard<std::mutex> locak(tasks_ptrs_mutex_);
        tasks_ptrs_[i] = local_tasks_.get();
      }
      auto id = std::hash<std::thread::id>{}(std::this_thread::get_id());
      while (true) {

        task_t task = nullptr;

        // 1. local queue
        {
          std::lock_guard<std::mutex> lock(local_tasks_->mutex);
          if (!local_tasks_->queue.empty()) {
            task = local_tasks_->queue.front();
            local_tasks_->queue.pop();
          }
        }

        // 2. global queue
        if (!task) {
          std::lock_guard<std::mutex> lock(mutex_);
          if (!tasks_.empty()) {
            task = tasks_.front();
            tasks_.pop();
          }
        }

        // 3. steal
        if (!task) {
          task = try_steal();
        }

        // 4. execute
        if (task) {
          task();
          continue;
        }

        // 5. wait
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
        if (stop_) {
          {
            std::lock_guard<std::mutex> lk(local_tasks_->mutex);
            // 检查本地任务
            if (!local_tasks_->queue.empty())
              continue;
          }
          // 检查全局队列
          if (!tasks_.empty()) {
            continue;
          }
          lock.unlock();
          // 检查其他线程队列
          task = try_steal();
          if (task) {
            task();
          } else {
            // 线程结束，移除队列指针，防止其他线程窃取悬空队列任务
            std::lock_guard<std::mutex> lock(tasks_ptrs_mutex_);
            auto it = std::find(tasks_ptrs_.begin(), tasks_ptrs_.end(),
                                local_tasks_.get());
            if (it != tasks_ptrs_.end())
              tasks_ptrs_.erase(it);
            break; // 都没有任务了，结束线程
          }
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

LThreadPool::task_t LThreadPool::try_steal() {
  task_t result{nullptr};
  std::mt19937 generator{std::random_device{}()};
  // 从随机位置开始偷取，避免热点偷取同一个线程，锁竞争大
  unsigned long start{0};
  {
    std::lock_guard<std::mutex> lock(tasks_ptrs_mutex_);
    auto size = tasks_ptrs_.size();
    if(size == 0){
      return result;
    }
    start = generator() % size;
    for (size_t i = 0; i < size; i++) {
      auto local_queue = tasks_ptrs_[(start + i) % size];
      // 不自己偷自己
      if (local_queue == local_tasks_.get()) {
        continue;
      }
      // 窃取任务
      {
        std::lock_guard<std::mutex> locK(local_queue->mutex);
        if (!local_queue->queue.empty()) {
          result = local_queue->queue.front();
          local_queue->queue.pop();
          break;
        }
      }
    }
  }
  return result;
}