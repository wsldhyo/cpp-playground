#ifndef UNIQUE_PTR_HPP
#define UNIQUE_PTR_HPP
#include <algorithm> // std::swap
#include <tuple>     // for EBO
#include <utility>   // std::exchange
namespace scratch {

/**
 * @brief UniquePtr删除器
 *   允许UniquePtr自定义删除行为
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
  constexpr UniquePtr() noexcept : data_(nullptr, deleter_type{}) {}
  // 移动函数
  constexpr UniquePtr(UniquePtr &&other) noexcept
      : data_(std::exchange(other.ptr(), nullptr), std::move(other.deleter())) {
  }

  constexpr UniquePtr &operator=(UniquePtr &&other) noexcept {
    if (this != &other) {
      if (ptr()) {
        get_deleter()(ptr());
      }
      ptr() = std::exchange(other.ptr(), nullptr);
      deleter() = std::move(other.deleter());
    }
    return *this;
  }
  // 析构
  ~UniquePtr() noexcept {
    if (ptr()) {
      get_deleter()(ptr());
    }
  }

  // 禁止拷贝
  UniquePtr(UniquePtr const &other) = delete;
  UniquePtr &operator=(UniquePtr const &other) = delete;

  // 转换构造
  constexpr explicit UniquePtr(pointer p) noexcept : data_(p, deleter_type{}) {}
  // 从nullptr的转换构造，允许隐式转换：UniquePtr<int> p = nullptr;
  constexpr UniquePtr(std::nullptr_t) noexcept
      : data_(nullptr, deleter_type{}) {}
  // 支持传递自定义删除器
  constexpr UniquePtr(pointer p, deleter_type d) noexcept
      : data_(p, std::move(d)) {}
  constexpr UniquePtr(std::nullptr_t, deleter_type d) noexcept
      : data_(nullptr, std::move(d)) {}

  // 获取裸指针
  constexpr pointer get() const noexcept { return ptr(); }
  constexpr deleter_type &get_deleter() noexcept { return deleter(); }
  constexpr deleter_type const &get_deleter() const noexcept {
    return deleter();
  }
  // 取消托管
  pointer release() noexcept { return std::exchange(ptr(), nullptr); }
  // 托管另一个指针
  void reset(pointer p = nullptr) noexcept {
    if (p != ptr()) {
      pointer old_obj = std::exchange(ptr(), p);
      if (old_obj) {
        get_deleter()(old_obj);
      }
    }
  }

  void swap(UniquePtr &other) noexcept {
    // 交换整个tuple
    std::swap(data_, other.data_);
  }

  // 转换函数
  constexpr explicit operator bool() const noexcept { return ptr() != nullptr; }

  // =============运算符重载=============
  // 解引用
  constexpr pointer operator->() const noexcept { return ptr(); }
  constexpr element_type &operator*() const noexcept { return *ptr(); }
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
    return lhs.ptr() == nullptr;
  }
  friend bool operator!=(const UniquePtr &lhs, std::nullptr_t) noexcept {
    return lhs.ptr() != nullptr;
  }
  friend bool operator==(std::nullptr_t, const UniquePtr &rhs) noexcept {
    return rhs.ptr() == nullptr;
  }
  friend bool operator!=(std::nullptr_t, const UniquePtr &rhs) noexcept {
    return rhs.ptr() != nullptr;
  }

private:
  /*
    主动继承实现EBO
      class UniquePtr : private Deleter {
        T* ptr_;
        public:
          ~UniquePtr(){*this(ptr_);}
      };
    但继承要求Deleter必须是类类型，其他可调用对象就无法作为删除器了，因此用tuple:
      tuple递归继承展开，可以自动优化掉data_中delete_type, 实现EBO(空基类优化)
  */
  std::tuple<pointer, deleter_type> data_;

  // 辅助访问函数, 获取data_中的数据
  pointer &ptr() noexcept { return std::get<0>(data_); }
  pointer const &ptr() const noexcept { return std::get<0>(data_); }
  deleter_type &deleter() noexcept { return std::get<1>(data_); }
  const deleter_type &deleter() const noexcept { return std::get<1>(data_); }
};

template <typename U, typename D>
bool operator==(const UniquePtr<U, D> &lhs,
                const UniquePtr<U, D> &rhs) noexcept {
  return lhs.ptr() == rhs.ptr();
}
} // namespace scratch
#endif // UNIQUE_PTR_HPP