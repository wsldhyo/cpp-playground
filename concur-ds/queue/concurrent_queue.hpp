#ifndef CONCURRENT_QUEUE_HPP
#define CONCURRENT_QUEUE_HPP

#include "../utils/noncp_nonmv.hpp"
#include <condition_variable>
#include <memory>
#include <mutex>
template <typename T>
class ConcurrentQueue : protected NonCopyable, protected NonMovable {
  struct Node {
    std::unique_ptr<T> data;
    std::unique_ptr<Node> next;
    Node() = default;
    Node(T const &val) : data(std::make_unique<T>(val)), next(nullptr) {}
    Node(T &&val) : data(std::make_unique<T>(std::move(val))), next(nullptr) {}
    ~Node() = default;
  };

public:
  // 初始分配head_， head_作为哑节点，tail_指向head_
  ConcurrentQueue() : head_(std::make_unique<Node>()), tail_(head_.get()) {}
  ~ConcurrentQueue() = default;

  template <typename U> void push(U &&val) {
    auto new_node = std::make_unique<Node>(std::forward<U>(val));
    {
      std::lock_guard<std::mutex> tail_lock(tail_mtx_);
      tail_->data =
          std::move(new_node->data); // new_node作为dummy node，不存储数据
      tail_->next = std::move(new_node);
      tail_ = tail_->next.get();
    }
    non_empty_cond_.notify_one();
  }

  bool try_pop(T &val) {
    {
      auto old_head = std::unique_ptr<Node>();
      {
        std::lock_guard<std::mutex> head_lock(head_mtx_);
        if (head_.get() == get_tail()) {
          return false;
        }
        old_head = std::move(head_);
        head_ = std::move(old_head->next);
      }
      val = std::move(*(old_head->data));
      return true;
    }
  }

  std::unique_ptr<T> try_pop() {
    auto old_head = std::unique_ptr<Node>();
    {
      std::lock_guard<std::mutex> head_lock(head_mtx_);
      if (head_.get() == get_tail()) {
        return std::move(old_head);
      }
      old_head = std::move(head_);
      head_ = std::move(old_head->next);
    }
    return std::move(old_head->data);
  }

  void wait_pop(T &val) {
    std::unique_ptr<Node> old_head;
    {
      std::unique_lock<std::mutex> lock(head_mtx_);
      non_empty_cond_.wait(lock,
                           [this]() { return head_.get() != get_tail(); });
      old_head = std::move(head_);
      head_ = std::move(old_head->next);
    }
    val = std::move(old_head->data);
  }

  bool empty() const {
    // 下面的写法，会同时持有两把锁，不符合设计目标
    // std::lock(head_mtx_, tail_mtx_);
    // std::lock_guard<std::mutex> head_lock(head_mtx_, std::adopt_lock);
    // std::lock_guard<std::mutex> tail_lock(tail_mtx_, std::adopt_lock);
    // return head_.get() == tail_;

    // empty不是强一致性的，不保证调用返回时队列仍为空，只是某个时刻观察队列为空
    // 因此pop时不用此empty函数判断空
    //
    std::lock_guard<std::mutex> lock(head_mtx_);
    return head_.get() == get_tail();
  }

private:
  Node *get_tail() const {
    std::lock_guard<std::mutex> tail_lock(tail_mtx_);
    return tail_;
  }
  // 队头弹出数据时，自动销毁Node节点
  std::unique_ptr<Node> head_;
  mutable std::mutex head_mtx_;
  // 只有一个数据时，头尾指针指向相同，但所有权只能归一个，因此为裸指针
  Node *tail_;
  mutable std::mutex tail_mtx_;
  std::condition_variable non_empty_cond_;
};

#endif // CONCURRENT_QUEUE_HPP
