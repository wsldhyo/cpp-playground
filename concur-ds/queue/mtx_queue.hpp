#ifndef MTX_QUEUE_HPP
#define MTX_QUEUE_HPP
#include "../utils/noncp_nonmv.hpp"
#include <condition_variable>
#include <deque>
#include <mutex>

template <typename T>
class MtxQueue : protected NonCopyable, protected NonMovable {
public:
  MtxQueue() = default;
  ~MtxQueue() = default;

  void push(T const &val) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      deque_.push_back(val);
    }
    cond_.notify_one();
  }

  void push(T &&val) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      deque_.push_back(std::move(val));
    }
    cond_.notify_one();
  }

  bool pop(T &val) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if(deque_.empty()){
        return false;
      }
      val = std::move(deque_.front());
      deque_.pop_front();
    }
    return false;
  }

  void wati_pop(T &val) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      cond_.wait(lock, [this](){
        return !deque_.empty();
      });
      val = std::move(deque_.front());
      deque_.pop_front();
    }
  }
private:
  std::deque<T> deque_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

#endif // MTX_QUEUE_HPP