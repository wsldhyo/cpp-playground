#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include "work_steal_queue.hpp"
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>
class LThreadPool {
public:
  using task_t = std::function<void()>; // 任务类型
  explicit LThreadPool(
      std::size_t thread_num = std::thread::hardware_concurrency());
  ~LThreadPool();

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

    auto wrapper = [task]() { (*task)(); };

    // task放入任务队列
    if (local_tasks_ != nullptr) {
      local_tasks_->push(std::move(wrapper));
      // cond_.notify_one(); 本地队j列，无需通知其他线程
      return result;
    }
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (stop_) {
        // 线程停止状态禁止入队
        std::cout << "Thread pool has stopped!submit task failed!\n";
        return std::future<return_type>(); // 返回一个空 future
      }
      // 放到临界区外的话，有可能push到全局队列前，中途其他线程将stop置为true，破坏stop_后不放任务的语义
      global_tasks_.push(std::move(wrapper));
    }
    cond_.notify_one();
    return result;
  }

private:
  bool try_steal_tasks();

  std::vector<std::thread> pool_;
  std::mutex mutex_; // 多个线程访问，需要保护tasks_
  WorkStealQue<task_t> global_tasks_;
  /// 本地队列, 指针确保非工作线程不会创建本地队列的实例
  std::vector<std::unique_ptr<WorkStealQue<task_t>>> local_tasks_queue_;
  thread_local static WorkStealQue<task_t> *local_tasks_;
  /// 通知有任务到达，分配线程执行任务
  std::condition_variable cond_;

  bool stop_{false};
};

#endif // THREAD_POOL_HPP
