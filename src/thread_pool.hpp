#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
class ThreadPool {
public:
  using task_t = void (*)(); // 任务函数类型
  explicit ThreadPool(std::size_t thread_num = std::thread::hardware_concurrency());
  ~ThreadPool();
  void submit(task_t task);

private:
  std::vector<std::thread> pool_;
  std::mutex mutex_;  // 多个线程访问，需要保护tasks_
  std::queue<task_t> tasks_;
  std::condition_variable cond_; // 通知有任务到达，分配线程执行任务
   
  bool stop_; // 线程池停止标记
};
#endif // THREAD_POOL_HPP