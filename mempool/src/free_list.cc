#include "free_list.hpp"
#include <cassert>
namespace lmem {

void *&next_memptr(void *obj) {
  // (void**)obj是将obj当做指向void*指针p2的指针p1，即将obj指向的内存块当做p2
  // *(void**)obj，就是解引用p1得到p2
  // p2存储在内存块上，不用担心生命周期问题
  return *(void **)obj;
}

void FreeList::push(void *obj) {
  if (obj == nullptr) {
    return;
  }

  next_memptr(obj) = list_head_; // 将obj添加到list_head_指向的内存块之前
  list_head_ = obj;
  ++size_;
}

void FreeList::push(void *start, void *end, std::size_t num) {
  next_memptr(end) = list_head_;
  list_head_ = start;
  size_ += num;
}


void *FreeList::pop() {
  assert(list_head_);
  auto res = list_head_;
  list_head_ =
      next_memptr(list_head_); // 头删，list_head_指向第一块内存的下一内存
  --size_;
  return res;
}

  void FreeList::pop(void*&start, void*end, std::size_t num){
    assert(num <= size_);
    start = list_head_;
    end = list_head_;
    for (auto i = 0; i < num - 1; ++i) {
      end = next_memptr(end);
    }
    list_head_ = next_memptr(end);
    next_memptr(end) = nullptr;
    size_ -= num;
  }

  std::size_t FreeList::size()const{
    return size_;
  }
bool FreeList::empty() const { return list_head_ == nullptr; }
} // namespace lmem