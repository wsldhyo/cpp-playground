#ifndef UNIQUE_PTR_HPP
#define UNIQUE_PTR_HPP
#include <algorithm> // std::swap
#include <utility> // std::exchange
namespace scratch {
template <typename T> class UniquePtr {
public:
  UniquePtr() = default;

  explicit UniquePtr(T *p) : obj_(p) {}

  ~UniquePtr() { delete obj_; }

  UniquePtr(UniquePtr &&other) noexcept
      : obj_(std::exchange(other.obj_, nullptr)) {}

  UniquePtr &operator=(UniquePtr &&other) noexcept {
    if (this != &other) {
      delete obj_;
      obj_ = std::exchange(other.obj_, nullptr);
    }
    return *this;
  }

  UniquePtr(UniquePtr const &other) = delete;
  UniquePtr &operator=(UniquePtr const &other) = delete;

  T *get() const { return obj_; }
  T *release() { return std::exchange(obj_, nullptr); }
  void reset(T *p = nullptr) {
    if (p != obj_) {
      T *old_obj = std::exchange(obj_, p);
      delete old_obj;
    }
  }
  void swap(UniquePtr<T> &other) { std::swap(obj_, other.obj_); }

private:
  T *obj_{nullptr};
};
} // namespace scratch
#endif // UNIQUE_PTR_HPP