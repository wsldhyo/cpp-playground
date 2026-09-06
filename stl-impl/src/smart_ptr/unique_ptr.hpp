#ifndef UNIQUE_PTR_HPP
#define UNIQUE_PTR_HPP
#include <algorithm> // std::swap
#include <utility>   // std::exchange
namespace scratch {

/**
 * @brief UniquePtr删除器
 *   允许UniquePtr自定义删除行为
 * @tparam T
 */
template <typename T> struct default_deleter {
  constexpr default_deleter() noexcept = default;

  void operator()(T *ptr) const {
    // NOLINTNEXTLINE(bugprone-sizeof-expression)：屏蔽cland警告
    static_assert(sizeof(T) > 0, "can't delete an incomplete type");
    // 对void*执行delete操作，无法得知类型大小，即无法得知释放内存大小
    static_assert(!std::is_void_v<T>, "can't delete an incomplete type");
    delete ptr;
  }
};

template <typename T, typename Deleter_t = default_deleter<T>> class UniquePtr {
public:
  // 类型别名
  using pointer = T *;
  using element_type = T;
  using deleter_type = Deleter_t;
  // 默认构造，删除器没有默认构造时则不提供默认构造
  template <typename D = deleter_type,
            typename = std::enable_if_t<std::is_default_constructible_v<D>>>
  constexpr UniquePtr() noexcept : obj_(nullptr), deleter_() {}
  // 移动函数
  constexpr UniquePtr(UniquePtr &&other) noexcept
      : obj_(std::exchange(other.obj_, nullptr)),
        deleter_(std::move(other.deleter_)) {}

  constexpr UniquePtr &operator=(UniquePtr &&other) noexcept {
    if (this != &other) {
      if (obj_) {
        get_deleter()(obj_);
      }
      obj_ = std::exchange(other.obj_, nullptr);
      deleter_ = std::move(other.deleter_);
    }
    return *this;
  }

  ~UniquePtr() noexcept {
    if (obj_) {
      get_deleter()(obj_);
    }
  }

  // 禁止拷贝
  UniquePtr(UniquePtr const &other) = delete;
  UniquePtr &operator=(UniquePtr const &other) = delete;

  // 转换构造
  constexpr explicit UniquePtr(pointer p) noexcept : obj_(p), deleter_() {}
  // 从nullptr的转换构造，允许隐式转换：UniquePtr<int> p = nullptr;
  constexpr UniquePtr(std::nullptr_t) noexcept : obj_(nullptr), deleter_() {}
  // 支持传递自定义删除器
  constexpr UniquePtr(pointer p, deleter_type d) noexcept
      : obj_(p), deleter_(std::move(d)) {}
  constexpr UniquePtr(std::nullptr_t, deleter_type d) noexcept
      : obj_(nullptr), deleter_(std::move(d)) {}

  // 获取裸指针
  constexpr pointer get() const noexcept { return obj_; }
  constexpr deleter_type &get_deleter() noexcept { return deleter_; }
  constexpr deleter_type const &get_deleter() const noexcept {
    return deleter_;
  }
  // 取消托管
  pointer release() noexcept { return std::exchange(obj_, nullptr); }
  // 托管另一个指针
  void reset(pointer p = nullptr) noexcept {
    if (p != obj_) {
      T *old_obj = std::exchange(obj_, p);
      get_deleter()(old_obj);
    }
  }

  void swap(UniquePtr &other) noexcept {
    std::swap(obj_, other.obj_);
    std::swap(deleter_, other.deleter_);
  }

  // 转换函数
  constexpr explicit operator bool() const noexcept { return obj_ != nullptr; }

  // =============运算符重载=============
  // 解引用
  constexpr pointer operator->() const noexcept { return obj_; }
  constexpr element_type &operator*() const noexcept { return *obj_; }
  // nullptr赋值，返回UniquePtr以支持链式赋值
  UniquePtr &operator=(std::nullptr_t) noexcept {
    reset();
    return *this;
  }
  // Unique_ptr之间比较相等
  template <typename U, typename D> // 友元实现在类外
  friend bool operator==(const UniquePtr<U, D> &lhs,
                         const UniquePtr<U, D> &rhs) noexcept;
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
  pointer obj_;
  deleter_type deleter_;
};

template <typename U, typename D>
bool operator==(const UniquePtr<U, D> &lhs,
                const UniquePtr<U, D> &rhs) noexcept {
  return lhs.obj_ == rhs.obj_;
}
} // namespace scratch
#endif // UNIQUE_PTR_HPP