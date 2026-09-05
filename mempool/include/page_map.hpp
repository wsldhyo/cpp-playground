#ifndef PAGE_MAP1_HPP
#define PAGE_MAP1_HPP
#include "global_var.hpp"
#include "size_class.hpp"
#include "sys_alloc.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
namespace lmem {
// 单层基数树
template <int Bits> class PageMap1 {
public:
  using Number = uintptr_t;
  PageMap1() {
    std::size_t size = sizeof(void *) << Bits;
    std::size_t aligned_size = SizeClass::round_up(size);
    array_ = (void **)system_allocate(aligned_size >> PAGE_SHIFT);
    memset(array_, 0, ARR_LENGTH);
  }

    void* get(Number k) const{
        if(k >> Bits > 0)
        {
            return nullptr;
        }
        return array_[k];
    }

    void set(Number k, void* v){
        if (k > ARR_LENGTH || v == nullptr) {
            return;
        }
        array_[k] = v;
    }
private:
  static int const ARR_LENGTH = 1 << Bits;
  void **array_;
};



} // namespace lmem
#endif