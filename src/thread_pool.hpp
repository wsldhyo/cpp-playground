#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>
class ThreadPool {
public:
  using task_t = std::function<void()>; // 任务类型
  explicit ThreadPool(
      std::size_t thread_num = std::thread::hardware_concurrency());
  ~ThreadPool();

  template <typename Func_t, typename... Args>
  auto submit(Func_t &&f, Args &&...args) -> std::future<
      std::invoke_result_t<std::decay_t<Func_t>, std::decay_t<Args>...>> {

    // 任务返回类型
    using return_type =
        std::invoke_result_t<std::decay_t<Func_t>, std::decay_t<Args>...>;
    // C++11/14可用std::result_of， 如下
    //    using return_type =
    //     typename std::result_of<
    //         typename std::decay<Func_t>::type(
    //             typename std::decay<Args>::type...
    //         )
    //     >::type;

    // 包装task
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        [f = std::forward<Func_t>(f),
         ... args = std::forward<Args>(args)]() mutable {
          return std::invoke(f, args...);
          // C++11/14用此种方式包装，则需要手写std::apply调用，简单起见用std::bind即可
        });

    // 也可用std::bind绑定任务和参数，如下
    // auto task = std::make_shared<std::packaged_task<return_type()>>(
    //     std::bind(std::forward<Func_t>(f), std::forward<Args>(args)...));

    std::future<return_type> result = task->get_future();

    // task放入任务队列
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (stop_) {
        // 线程停止状态禁止入队
        std::cout << "Thread pool has stopped!submit task failed!\n";
        return std::future<return_type>(); // 返回一个空 future
      }

      tasks_.emplace([task]() { (*task)(); });
    }

    cond_.notify_one();
    return result;
  }

private:
  std::vector<std::thread> pool_;
  std::mutex mutex_; // 多个线程访问，需要保护tasks_
  std::queue<task_t> tasks_;
  std::condition_variable cond_; // 通知有任务到达，分配线程执行任务

  bool stop_; // 线程池停止标记
};


#endif // THREAD_POOL_HPP