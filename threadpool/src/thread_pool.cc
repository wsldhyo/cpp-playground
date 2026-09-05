#include "thread_pool.hpp"
#include "work_steal_queue.hpp"
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

thread_local WorkStealQue<LThreadPool::task_t> *LThreadPool::local_tasks_{
    nullptr};
LThreadPool::LThreadPool(std::size_t thread_num) : stop_(false) {
  local_tasks_queue_.resize(thread_num);
  for (std::size_t i = 0; i < thread_num; ++i) {
    local_tasks_queue_[i] = (std::make_unique<WorkStealQue<task_t>>());
    // 构造任务线程，尝试从池中取出任务并执行
    pool_.emplace_back([this, i]() {
      local_tasks_ = local_tasks_queue_[i].get();
      while (true) {

        task_t task = nullptr;
        // 1. 本地队列尾部 pop
        if (!local_tasks_->try_pop(task)) {

          // 2. 全局队列尾部 pop
          if (!global_tasks_.try_pop(task)) {

            // 3. 从其他线程头部 steal
            if (try_steal_tasks()) {
              continue;
            }
          }
        }
        // 4. execute
        if (task) {
          task();
          continue;
        }

        // 5. wait
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return stop_ || !global_tasks_.empty(); });
        if (stop_) {
          {
            // 检查本地任务
            if (!local_tasks_->empty())
              continue;
          }
          // 检查全局队列
          if (!global_tasks_.empty()) {
            continue;
          }
          lock.unlock();
          // 检查其他线程队列
          if (try_steal_tasks()) {
            continue; // 窃取到了其他线程的任务
          }
          break; // 其他线程也没有任务
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

bool LThreadPool::try_steal_tasks() {
  task_t result{nullptr};
  thread_local std::mt19937 generator{std::random_device{}()};
  // 从随机位置开始偷取，避免热点偷取同一个线程，锁竞争大
  unsigned long start{0};
  unsigned long size{0};

  size = local_tasks_queue_.size();
  if (size == 0) {
    return false;
  }
  start = generator() % size;
  std::vector<task_t> tasks;
  for (size_t i = 0; i < size; i++) {
    auto &local_queue = local_tasks_queue_[(start + i) % size];
    if (!local_queue)
      continue;
    if (local_tasks_ == local_queue.get()) // 不自己偷自己
      continue;
    if (local_queue->try_steal(tasks)) {
      local_tasks_->push(std::move(tasks));
      return true;
    }
  }
  return false;
}