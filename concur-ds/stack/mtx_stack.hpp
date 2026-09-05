#ifndef MTX_STACK_HPP
#define MTX_STACK_HPP
#include "../utils/noncp_nonmv.hpp"
#include <condition_variable>
#include <mutex>
#include <stack>
#define LOCK_GUARD(mtx) std::lock_guard<std::mutex> lock(mtx);
template <typename T>
class MtxStack : protected NonCopyable, protected NonMovable {
public:
  MtxStack() = default;
  ~MtxStack() = default;

  template <typename U> void push(U &&val) {
    {
      LOCK_GUARD(mutex_)
      stack_.push(std::forward<U>(val));
    }
    cond_.notify_one();
  }

  bool pop(T &val) {
    {
      LOCK_GUARD(mutex_)
      if (stack_.empty()) {  // 注意判空 
        return false;
      }
      val = std::move(stack_.top()); // move减少拷贝
      stack_.pop();
    }
    return true;
  }

  void wait_pop(T &val) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      cond_.wait(lock, [this]() { return !stack_.empty(); });
      val = std::move(stack_.top()); // move减少拷贝
      stack_.pop();
    }
  }

  std::size_t size() const {
    LOCK_GUARD(mutex_)
    return stack_.size();
  }

  bool empty() const {
    LOCK_GUARD(mutex_)
    return stack_.empty();
  }

private:
  mutable std::mutex mutex_;
  std::stack<T> stack_;
  std::condition_variable cond_;
};

#endif // MTX_STACK_HPP