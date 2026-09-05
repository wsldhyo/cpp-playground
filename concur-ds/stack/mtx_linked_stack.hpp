#ifndef LINKED_STACK_HPP
#define LINKED_STACK_HPP
#include "../utils/noncp_nonmv.hpp"
#include <cassert>
#include <condition_variable>
#include <memory>
#include <mutex>

template <typename T>
class MtxLinkedStack : protected NonCopyable, protected NonMovable {
  struct Node : protected NonCopyable {
    T value;
    std::unique_ptr<Node> next;
    ~Node() = default;
    template <typename U>
    explicit Node(U &&val) : value(std::forward<U>(val)), next(nullptr) {}
  };

public:
  MtxLinkedStack() = default;
  ~MtxLinkedStack() = default;

  template <typename U> void push(U &&val) {
    auto new_node = std::make_unique<Node>(std::forward<U>(val));
    {
      std::lock_guard<std::mutex> lock(mtx_);
      new_node->next = std::move(top_); // 头插链表
      top_ = std::move(new_node);
    }
    cond_.notify_one();
  }

  bool pop(T &val) {
    std::unique_ptr<Node> old_top{nullptr};
    {
      std::lock_guard<std::mutex> lock(mtx_);
      if (top_ == nullptr) { // 注意判空
        return false;
      }
      old_top = std::move(top_);
      top_ = std::move(old_top->next);
    }
    val = std::move(old_top->value); // move减少拷贝
    return true;
  }

  void wait_pop(T &val) {
    std::unique_ptr<Node> old_top{nullptr};
    {
      std::unique_lock<std::mutex> lock(mtx_);
      cond_.wait(lock, [this]() { return top_ != nullptr; });
      old_top = std::move(top_);
      top_ = std::move(old_top->next);
    }
    val = std::move(old_top->value); // move减少拷贝
  }


  bool empty() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return top_ == nullptr;
  }

private:
  std::unique_ptr<Node> top_{nullptr};
  mutable std::mutex mtx_;
  std::condition_variable cond_;
};
#endif // LINKED_STACK_HPP