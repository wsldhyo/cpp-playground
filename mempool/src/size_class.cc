#include <cassert>
#include <cstddef>
#include "global_var.hpp"
#include "size_class.hpp"
namespace lmem {
std::size_t SizeClass::round_up(std::size_t size) {
  if (size <= 128) {
    return round_up_(size, 8);
  } else if (size <= 1024) {
    return round_up_(size, 16);
  } else if (size <= 8 * 1024) {
    return round_up_(size, 128);
  } else if (size <= 64 * 1024) {
    return round_up_(size, 1024);
  } else if (size <= 256 * 1024) {
    return round_up_(size, 8 * 1024);
  } else {
    assert(false);
    return -1;
  }
}

std::size_t SizeClass::round_up_(std::size_t size, std::size_t align_val) {
  std::size_t res = size;
  if (size % align_val) {
    // 有余数，整除后+1, 再乘align_val，可以得到比size稍大，且是align_val的倍数
    res = (size / align_val + 1) * align_val;
  }
  return res;
}

std::size_t SizeClass::get_hash_index(std::size_t size)

{
  assert(size <= MAX_TC_BLOCK_SIZE);
  static int range_list_nums[4] = {16, 72, 128, 184};
  if (size <= 128) {
    return get_hash_index_(size, 3);
  } else if (size <= 1024) {
    return get_hash_index_(size - 128, 4) + range_list_nums[0];
  } else if (size <= 8 * 1024) {
    return get_hash_index_(size - 1024, 7) + range_list_nums[1];
  } else if (size <= 64 * 1024) {
    return get_hash_index_(size - 8 * 1024, 10) + range_list_nums[2];
  } else if (size <= 256 * 1024) {
    return get_hash_index_(size - 64 * 1024, 13) + range_list_nums[3];
  } else {
    assert(false);
  }
  return -1;
}

std::size_t SizeClass::get_hash_index_(std::size_t size,
                                       std::size_t align_shift) {
  // 1 << align_shift是对齐值
  // size + (1 << align_shift) - 1 是将 size 调整到大于size最近对齐值的数（但并
  // 未直接对齐）, 为后面计算出size相对于对齐值的倍数做准备
  // >> align_shitf 即将调整后的值整除对齐值，得到倍数
  // 最后-1得到以0开始的索引

  // 例如size = 10， align_shitf = 3, 调整后为10 + (2 << 3) - 1
  // , 即调整为17（对齐值为8时，10的最近对齐值为16）。
  // 然后17 / 8 - 1 = 1，因此size=10位于第二个哈希桶
  
  return ((size + (1 << align_shift) - 1) >> align_shift) - 1;
}

std::size_t SizeClass::count_block_num(std::size_t size) {
  assert(size > 0);
  // 除一下，计算每种size对应的个数
  std::size_t num = MAX_TC_BLOCK_SIZE / size;
  
  // 限制内存块数的最大值和最小值
  if (MAX_TC_BLOCK_SIZE / size > 512) {
    num = 512;
  } 
  else if(num < 2)
  {
    num = 2;
  }
  return num;
}


   std::size_t SizeClass::count_page_num(std::size_t size){
    // 获取Tc向CC申请时，单次最大申请数
     auto num = count_block_num(size); 
    
    //计算单次申请大小，再除以页大小，即得申请页数
    std::size_t npage = num * size;
    npage >>= PAGE_SHIFT;
    
    // 至少申请一页内存
    if (npage == 0) {
      npage = 1;
    }
    return npage;
   }
} // namespace lmem