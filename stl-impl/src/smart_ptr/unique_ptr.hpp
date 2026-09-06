#ifndef UNIQUE_PTR_HPP
#define UNIQUE_PTR_HPP
#include <algorithm> // std::swap
#include <utility>   // std::exchange
namespace scratch {

template <typename T> class UniquePtr {
public:
  // 类型别名
  using pointer = T *;
  using element_type = T;

  constexpr UniquePtr() noexcept = default;

  // 转换构造
  constexpr explicit UniquePtr(pointer p) noexcept : obj_(p) {}
  // 从nullptr的转换构造，允许隐式转换：UniquePtr<int> p = nullptr;
  constexpr UniquePtr(std::nullptr_t) noexcept : obj_(nullptr) {}

  constexpr UniquePtr(UniquePtr &&other) noexcept
      : obj_(std::exchange(other.obj_, nullptr)) {}

  constexpr UniquePtr &operator=(UniquePtr &&other) noexcept {
    if (this != &other) {
      delete obj_;
      obj_ = std::exchange(other.obj_, nullptr);
    }
    return *this;
  }

  ~UniquePtr() noexcept { delete obj_; }

  // 禁止拷贝
  UniquePtr(UniquePtr const &other) = delete;
  UniquePtr &operator=(UniquePtr const &other) = delete;
  // 获取裸指针
  constexpr pointer get() const noexcept { return obj_; }
  // 取消托管
  pointer release() noexcept { return std::exchange(obj_, nullptr); }
  // 托管另一个指针
  void reset(pointer p = nullptr) noexcept {
    if (p != obj_) {
      T *old_obj = std::exchange(obj_, p);
      delete old_obj;
    }
  }

  void swap(UniquePtr &other) noexcept { std::swap(obj_, other.obj_); }

  // 转换函数
  constexpr explicit operator bool() const noexcept { return obj_ != nullptr; }

  // =============运算符重载=============
  // 解引用
  constexpr pointer operator->() const noexcept { return obj_; }
  constexpr element_type &operator*() const noexcept { return *obj_; }
  // nullptr赋值，返回UniquePtr以支持链式赋值
  UniquePtr &operator=(std::nullptr_t) noexcept { reset(); }
  // Unique_ptr之间比较相等
  template <typename U>  // 友元实现在类外
  friend bool operator==(const UniquePtr<U> &lhs,
                         const UniquePtr<U> &rhs) noexcept;
  friend bool operator!=(const UniquePtr &lhs, const UniquePtr &rhs) noexcept {
    return !(lhs == rhs);
  }
  // UniquePtr与nullptr比较相等
  friend bool operator==(const UniquePtr &lhs, std::nullptr_t) noexcept {
    return lhs.obj_ == nullptr;
  }
  friend bool operator!=(const UniquePtr &lhs, std::nullptr_t) noexcept {
    return lhs.obj_ != nullptr;
  }
  friend bool operator==(std::nullptr_t, const UniquePtr &rhs) noexcept {
    return rhs.obj_ == nullptr;
  }
  friend bool operator!=(std::nullptr_t, const UniquePtr &rhs) noexcept {
    return rhs.obj_ != nullptr;
  }

private:
  pointer obj_{nullptr};
};

template <typename U>
bool operator==(const UniquePtr<U> &lhs, const UniquePtr<U> &rhs) noexcept {
  return lhs.obj_ == rhs.obj_;
}
} // namespace scratch
#endif // UNIQUE_PTR_HPP