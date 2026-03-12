#ifndef WORK_STEAL_QUEUE_HPP
#define WORK_STEAL_QUEUE_HPP

#include <cstdarg>
#include <deque>
#include <mutex>
#include <vector>
/**
 * @brief 基于deque的并发队列，支持任务窃取
 *
 * @tparam T
 */
template <typename T> class WorkStealQue {
public:
  template <typename U> void push(U &&val) {
    std::lock_guard<std::mutex> lock(mutex_);
    deque_.emplace_back(std::forward<U>(val));
  }

  void push(std::vector<T> const &values) {
    std::lock_guard<std::mutex> lock(mutex_);
    deque_.insert(deque_.end(), values.begin(), values.end());
  }

  void push(std::vector<T> &&values) {
    std::lock_guard<std::mutex> lock(mutex_);
    deque_.insert(deque_.end(), std::make_move_iterator(values.begin()),
                  std::make_move_iterator(values.end()));
  }

  template <typename U> bool try_push(U &&val) {
    bool result{false};
    // 不用raii锁，并发下可能会重复进入
    if (!mutex_.try_lock()) {
      return result;
    }
    std::lock_guard<std::mutex> lock(
        mutex_, std::adopt_lock); // 异常发生时，也可以释放锁
    deque_.emplace_back(std::forward<U>(val));
    result = true;
    return result;
  }

  bool try_pop(T &val) {
    bool result{false};
    if (!mutex_.try_lock())
      return result;
    std::lock_guard<std::mutex> lock(
        mutex_,
        std::adopt_lock); // 异常发生时，也可以释放锁

    if (!deque_.empty()) {
      val = std::move(deque_.back());
      deque_.pop_back();
      result = true;
    }
    return result;
  }

  /**
   * @brief 尝试批量弹出任务。减少加锁次数，提高并发性
   *
   * @param res
   * @param max_batch_size
   * @return true
   * @return false
   */
  bool try_pop(std::vector<T> &values, std::size_t max_batch_size) {
    bool result = false;

    if (!mutex_.try_lock()) {
      return result;
    }
    std::lock_guard<std::mutex> lock(
        mutex_, std::adopt_lock); // 异常发生时，也可以释放锁

    values.reserve(values.size() + max_batch_size); // 获取锁后，再预分配内存
    while (!deque_.empty() && max_batch_size--) {
      values.emplace_back(std::move(deque_.back()));
      deque_.pop_back();
      result = true;
    }

    return result;
  }

  bool try_steal(T &val) {
    bool result{false};

    if (!mutex_.try_lock()) {
      return result;
    }
    std::lock_guard<std::mutex> lock(
        mutex_, std::adopt_lock); // 异常发生时，也可以释放锁

    if (!deque_.empty()) {
      val = std::move(deque_.front());
      deque_.pop_front();
      result = true;
    }
    return result;
  }

  bool try_steal(std::vector<T> &vals) {
    bool result{false};

    if (!mutex_.try_lock()) {
      return result;
    }
    {
      std::lock_guard<std::mutex> lock(mutex_, std::adopt_lock);

      size_t n = deque_.size() / 2; // 偷一半
      if (n == 0) {
        return false; // 元素太少，不偷
      }

      // 从前面偷 n 个
      auto begin = deque_.begin();
      auto end = begin + n;

      // 移动到 vals
      vals.insert(vals.end(), std::make_move_iterator(begin),
                  std::make_move_iterator(end));

      // 删除 deque 中的这些元素
      deque_.erase(begin, end);
    }
    result = true;
    return result;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return deque_.empty();
  }

private:
  std::deque<T> deque_;
  mutable std::mutex mutex_;
};

#endif // WORK_STEAL_QUEUE_HPP
