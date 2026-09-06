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

/**
 * @brief deleter的数组特化
 *  使用delete[] p删除而非delete p
 */
template <typename T> struct default_deleter<T[]> {
  constexpr default_deleter() noexcept = default;

  void operator()(T *ptr) const {
    // NOLINTNEXTLINE(bugprone-sizeof-expression)：屏蔽cland警告
    static_assert(sizeof(T) > 0, "can't delete an incomplete type");
    delete[] ptr;
  }
};

template <typename T, typename Deleter_t> class UniquePtrImpl {
public:
  // 类型别名
  using pointer = T *;
  using element_type = T;
  using deleter_type = Deleter_t;
  UniquePtrImpl() = default;
  // 转换构造，从裸指针构造
  constexpr explicit UniquePtrImpl(pointer p) noexcept
      : data_(p, deleter_type{}) {}
  // 带删除器构造
  template <typename D>
  constexpr UniquePtrImpl(pointer p, D &&d) noexcept
      : data_(p, std::forward<D>(d)) {}
  // 移动构造
  constexpr UniquePtrImpl(UniquePtrImpl &&other) noexcept
      : data_(std::move(other.data_)) {
    // 本对象为新构造对象，无需释放已有资源操作
    other.ptr() = nullptr; // 资源所有权已经被转移给新对象
  }

  constexpr UniquePtrImpl &operator=(UniquePtrImpl &&other) noexcept {
    if (this != &other) {
      reset(other.release()); // 要先释放本对象已有资源
      deleter() = std::forward<deleter_type>(other.deleter());
    }
    return *this;
  }

  // 禁止拷贝
  UniquePtrImpl(UniquePtrImpl const &other) = delete;
  UniquePtrImpl &operator=(UniquePtrImpl const &other) = delete;

  constexpr UniquePtrImpl(std::nullptr_t, deleter_type d) noexcept
      : data_(nullptr, std::move(d)) {}

  // 取消托管
  pointer release() noexcept { return std::exchange(ptr(), nullptr); }

  // 托管另一个指针
  void reset(pointer p = nullptr) noexcept {
    if (p != ptr()) {
      pointer old_obj = std::exchange(ptr(), p);
      if (old_obj) {
        deleter()(old_obj);
      }
    }
  }

  void swap(UniquePtrImpl &other) noexcept {
    // 交换整个tuple
    std::swap(ptr(), other.ptr());
    std::swap(deleter(), other.deleter());
  }

  // 辅助访问函数, 获取data_中的数据
  pointer &ptr() noexcept { return std::get<0>(data_); }
  pointer const &ptr() const noexcept { return std::get<0>(data_); }
  deleter_type &deleter() noexcept { return std::get<1>(data_); }
  const deleter_type &deleter() const noexcept { return std::get<1>(data_); }

private:
  /*
    主动继承实现EBO
      class UniquePtrImpl : private Deleter {
        T* ptr_;
        public:
          ~UniquePtrImpl(){*this(ptr_);}
      };
    但继承要求Deleter必须是类类型，其他可调用对象就无法作为删除器了，因此用tuple:
      tuple递归继承展开，可以自动优化掉data_中delete_type, 实现EBO(空基类优化)
  */
  std::tuple<pointer, deleter_type> data_;
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
  constexpr UniquePtr() noexcept : impl_(nullptr, deleter_type{}) {}
  // 移动函数, UniqueImpl已经实现资源移动，使用默认生成的即可
  UniquePtr(UniquePtr &&other) = default;
  UniquePtr &operator=(UniquePtr &&other) = default;
  // 析构
  ~UniquePtr() noexcept {
    if (impl_.ptr()) {
      get_deleter()(impl_.ptr());
    }
  }

  // 转换构造
  constexpr explicit UniquePtr(pointer p) noexcept : impl_(p, deleter_type{}) {}
  // 从nullptr的转换构造，允许隐式转换：UniquePtr<int> p = nullptr;
  constexpr UniquePtr(std::nullptr_t) noexcept
      : impl_(nullptr, deleter_type{}) {}
  // 支持传递自定义删除器
  constexpr UniquePtr(pointer p, deleter_type d) noexcept
      : impl_(p, std::move(d)) {}
  constexpr UniquePtr(std::nullptr_t, deleter_type d) noexcept
      : impl_(nullptr, std::move(d)) {}

  // 获取裸指针
  constexpr pointer get() const noexcept { return impl_.ptr(); }
  constexpr deleter_type &get_deleter() noexcept { return impl_.deleter(); }
  constexpr deleter_type const &get_deleter() const noexcept {
    return impl_.deleter();
  }
  // 取消托管
  pointer release() noexcept { return impl_.release(); }
  // 托管另一个指针
  void reset(pointer p = nullptr) noexcept { impl_.reset(p); }

  void swap(UniquePtr &other) noexcept { impl_.swap(other.impl_); }

  // 转换函数
  constexpr explicit operator bool() const noexcept {
    return impl_.ptr() != nullptr;
  }

  // =============运算符重载=============
  // 解引用
  constexpr pointer operator->() const noexcept { return get(); }
  constexpr element_type &operator*() const noexcept { return *get(); }
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
    return lhs.get() == nullptr;
  }
  friend bool operator!=(const UniquePtr &lhs, std::nullptr_t) noexcept {
    return lhs.get() != nullptr;
  }
  friend bool operator==(std::nullptr_t, const UniquePtr &rhs) noexcept {
    return rhs.get() == nullptr;
  }
  friend bool operator!=(std::nullptr_t, const UniquePtr &rhs) noexcept {
    return rhs.get() != nullptr;
  }

private:
  UniquePtrImpl<element_type, deleter_type> impl_;
};

template <typename U, typename D>
bool operator==(const UniquePtr<U, D> &lhs,
                const UniquePtr<U, D> &rhs) noexcept {
  return lhs.get() == rhs.get();
}

// 数组特化版本
// 禁用解引用->和*, 增加下标运算[]
template <typename T, typename Deleter_t> class UniquePtr<T[], Deleter_t> {
public:
  using pointer = T *;
  using element_type = T;
  using deleter_type = Deleter_t;

  // ---------- 构造函数 ----------
  // 默认构造（要求删除器可默认构造）
  template <typename D = deleter_type,
            typename = std::enable_if_t<std::is_default_constructible_v<D>>>
  constexpr UniquePtr() noexcept : impl_(nullptr, deleter_type{}) {}

  // 从裸指针构造
  constexpr explicit UniquePtr(pointer p) noexcept : impl_(p, deleter_type{}) {}

  // 从 nullptr 构造
  constexpr UniquePtr(std::nullptr_t) noexcept
      : impl_(nullptr, deleter_type{}) {}

  // 带自定义删除器
  constexpr UniquePtr(pointer p, deleter_type d) noexcept
      : impl_(p, std::move(d)) {}

  constexpr UniquePtr(std::nullptr_t, deleter_type d) noexcept
      : impl_(nullptr, std::move(d)) {}

  // 移动构造
  UniquePtr(UniquePtr &&other) noexcept = default;
  // 移动赋值
  UniquePtr &operator=(UniquePtr &&other) noexcept = default;

  // 禁止拷贝
  UniquePtr(const UniquePtr &) = delete;
  UniquePtr &operator=(const UniquePtr &) = delete;

  // 析构
  ~UniquePtr() {
    if (impl_.ptr()) {
      impl_.deleter()(impl_.ptr());
    }
  }

  // ---------- 修改器 ----------
  pointer release() noexcept { return impl_.release(); }

  void reset(pointer p = nullptr) noexcept { impl_.reset(p); }

  void swap(UniquePtr &other) noexcept { impl_.swap(other.impl_); }

  // ---------- 观察器 ----------
  constexpr pointer get() const noexcept { return impl_.ptr(); }
  constexpr deleter_type &get_deleter() noexcept { return impl_.deleter(); }
  constexpr const deleter_type &get_deleter() const noexcept {
    return impl_.deleter();
  }
  constexpr explicit operator bool() const noexcept { return get() != nullptr; }

  // ---------- 数组下标访问 ----------
  constexpr element_type &operator[](std::size_t idx) const noexcept {
    return get()[idx];
  }

  // 禁止解引用和箭头运算符
  // *ptr
  // 会返回数组的第一个元素，会让人误以为该智能指针管理的是单个T对象，而不是数组
  element_type &operator*() const = delete;
  pointer operator->() const = delete;

  // ---------- 与 nullptr 比较 ----------
  friend bool operator==(const UniquePtr &lhs, std::nullptr_t) noexcept {
    return lhs.get() == nullptr;
  }
  friend bool operator!=(const UniquePtr &lhs, std::nullptr_t) noexcept {
    return lhs.get() != nullptr;
  }
  friend bool operator==(std::nullptr_t, const UniquePtr &rhs) noexcept {
    return rhs.get() == nullptr;
  }
  friend bool operator!=(std::nullptr_t, const UniquePtr &rhs) noexcept {
    return rhs.get() != nullptr;
  }

private:
  UniquePtrImpl<element_type, deleter_type> impl_; // 复用原实现
};

// 单个对象
template <typename T, typename... Args,
          typename = std::enable_if_t<
              !std::is_array_v<T>>> // T 不能是数组类型如int[]、int[5]
UniquePtr<T> make_uniqueptr(Args... args) {
  return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

// 未定边界数组T[]，数组大小由函数参数指定
template <typename T, typename = std::enable_if_t<std::is_array_v<T> &&
                                                  std::extent_v<T> == 0>>
UniquePtr<T> make_uniqueptr(std::size_t size) {
  using E = std::remove_extent_t<T>;
  return UniquePtr<T>(new E[size]());
}

/*
 已知边界数组版本T[N]，数组大小由模板参数编译期指定：禁止
 UniquePtr只特化了UniquePtr<T[]>, 它与UniquePtr<T[N]>不是相同类型，原因如下
    构造 unique_ptr<T[N]> 需要传入一个指向固定大小为N的数组的指针，即 T(*)[N] 类型。而
    new T[N] 返回的是 T*

    make_uniqueptr返回的UniquePtr<T[N]>，
    不支持operator[]，因为UniquePtr内部指针类型是T(*)[N], 是指向整个数组的指针，对该
    指针的解引用得到整个数组，指针算术移动一个数组距离，而[n]等价于*(ptr + n)，与期望
    的第n个元素语义冲突
*/
template <typename T, typename = std::enable_if_t<std::is_array_v<T> &&
                                                  std::extent_v<T> != 0>>
UniquePtr<T> make_uniqueptr() = delete;
} // namespace scratch
#endif // UNIQUE_PTR_HPP