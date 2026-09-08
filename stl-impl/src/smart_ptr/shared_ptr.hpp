#ifndef SHARED_PTR_HPP
#define SHARED_PTR_HPP
#include <atomic>
#include <utility>
namespace scratch {

class SPtrRefCount {
public:
  SPtrRefCount() = default;
  void increase() noexcept {
    // 无需建立同步关系/约束重排，releaxed即可。
    // relaxed 不建立跨线程同步关系，但 fetch_add 仍是原子 RMW。
    // 所有修改按照该 atomic 对象的 modification order 排序。
    ref_count_.fetch_add(1, std::memory_order_relaxed);
  };

  std::size_t decrease() noexcept {
    /* decrease 可能导致资源释放（计数归零），因此需要 acq_rel。
     *
     * - Release ：保证当前线程在递减之前对对象的所有修改不会被重排到递减之后，
     *   从而能被后续其他线程的 acquire 操作观察到。
     *
     * - Acquire ：保证fetch_sub之后的操作（如 delete ptr_
     * 中的析构）不会被重排到fetch_sub之前， 并使当前线程（最后的
     * owner）与之前其他线程的 release 操作同步。保证之前所有的操作先于销毁
     *
     */
    return ref_count_.fetch_sub(1, std::memory_order_acq_rel);
  }

  std::size_t get_count() const noexcept {
    // 读取值即可，无需建立任何同步关系/约束重排
    return ref_count_.load(std::memory_order_relaxed);
  }

private:
  std::atomic_size_t ref_count_{1};
};

template <typename T> class SharedPtr {
public:
  using pointer = T *;
  using element_type = T;
  using reference = T &;
  using counter_t = SPtrRefCount *;

  SharedPtr() = default;
  explicit SharedPtr(pointer p) : ptr_(p), counter_(nullptr) {
    if (p == nullptr) {
      return;
    }
    try {
      counter_ = new SPtrRefCount();
    } catch (...) {
      // new发生异常，取消托管并释放资源，并将异常抛给上层处理
      delete p;
      throw;
    }
  }
  // 拷贝语义
  SharedPtr(SharedPtr const &rhs) noexcept
      : ptr_(rhs.ptr_), counter_(rhs.counter_) {
    incr_count();
  }

  SharedPtr &operator=(SharedPtr const &rhs) noexcept {
    if (this != &rhs) {
      rhs.incr_count();
      decr_count();
      ptr_ = rhs.ptr_;
      counter_ = rhs.counter_;
    }
    return *this;
  }

  // 移动语义
  SharedPtr(SharedPtr &&rhs) noexcept {
    ptr_ = std::exchange(rhs.ptr_, nullptr);
    counter_ = std::exchange(rhs.counter_, nullptr);
  }

  SharedPtr &operator=(SharedPtr &&rhs) noexcept {
    if (this != &rhs) {
      decr_count();
      ptr_ = std::exchange(rhs.ptr_, nullptr);
      counter_ = std::exchange(rhs.counter_, nullptr);
    }
    return *this;
  }

  // 析构
  ~SharedPtr() { decr_count(); }

  // 裸指针
  constexpr pointer get() const noexcept { return ptr_; }
  // ===========重置托管对象===========
  // 本对象放弃所有权，若是最后一个owner，则释放资源
  // 先构造新状态（控制块），成功后再提交(通过swap交换)。避免构造控制块时发生异常导致无法回收资源
  void reset() noexcept { SharedPtr().swap(*this); }

  // 托管另一指针
  template <typename U> void reset(U *p) {
    // 不能重新托管同一对象
    if (p != ptr_) {
      // 隐式检查 U* 的 p 是否可以转换到 T* 的 ptr_
      SharedPtr(p).swap(*this);
    }
  }

  // 交换
  void swap(SharedPtr &rhs) noexcept {
    std::swap(ptr_, rhs.ptr_);
    std::swap(counter_, rhs.counter_);
  }
  // bool转换
  explicit constexpr operator bool() const noexcept { return ptr_ != nullptr; }
  // 引用计数
  std::size_t use_count() const noexcept {
    return counter_ == nullptr ? 0 : counter_->get_count();
  }
  // 解引用
  constexpr pointer operator->() const noexcept { return ptr_; }

  constexpr element_type &operator*() const noexcept { return *ptr_; }

private:
  void incr_count() {
    if (counter_) {
      counter_->increase();
    }
  }

  void decr_count() {
    if (counter_) {
      if (counter_->decrease() == 1) {
        // 旧值为1, 减1后为0, 不再有人持有，释放资源
        delete ptr_;
        ptr_ = nullptr;
        delete counter_;
        counter_ = nullptr;
      }
    }
  }

  pointer ptr_{nullptr};
  counter_t counter_{nullptr};
};

template <typename T> class WeakdPtr {};

template <typename T> class EnableSharedFromThis {};

template <typename T> SharedPtr<T> make_sharedptr();
} // namespace scratch
#endif // SHARED_PTR_HPP