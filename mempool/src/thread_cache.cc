#include "thread_cache.hpp"
#include "central_cache.hpp"
#include "free_list.hpp"
#include "global_var.hpp"
#include "size_class.hpp"
#include <cassert>

namespace lmem {


void *ThreadCache::allocate(std::size_t size) {
  auto aligned_size = SizeClass::round_up(size);
  assert(aligned_size <= MAX_TC_BLOCK_SIZE);
  auto hash_index = SizeClass::get_hash_index(size);
  if (!free_lists_[hash_index].empty()) {
    return free_lists_[hash_index].pop();
  } else {
    return fetch_mem_from_cc(hash_index, aligned_size);
  }
}

void ThreadCache::deallocate(void *obj, std::size_t aligned_size) {
  if (obj == nullptr) {
    return;
  }
  assert(aligned_size <= MAX_TC_BLOCK_SIZE);
  auto hash_index = SizeClass::get_hash_index(aligned_size);
  auto& free_list = free_lists_[hash_index];
  free_list.push(obj);
  if (free_list.size() >= fetch_mem_nums_[hash_index]) {
    return_mem_to_cc(free_list, fetch_mem_nums_[hash_index]);
    fetch_mem_nums_[hash_index] = 1;
  }
}

void *ThreadCache::fetch_mem_from_cc(std::size_t hash_index,
                                     std::size_t aligned_size) {
  if (fetch_mem_nums_[hash_index] == 0) {
    fetch_mem_nums_[hash_index] = 1;
  }
  auto alloc_nums =
      SizeClass::count_block_num(aligned_size);
  alloc_nums = std::min(alloc_nums, fetch_mem_nums_[hash_index]);
  // 如果单次申请数量没有达到上限
  // 则增加fetch_mem_nums_[hash_index]
  // 以便让下次申请相同大小内存时，向CC申请更多块内存
  if (alloc_nums  == fetch_mem_nums_[hash_index]) {
    expand_fetch_nums(hash_index);
  }

  CentralCache *CC = CentralCache::get_instance();
  void *start;
  void *end;
  auto actual_num = CC->fetch_batch_mems(start, end, alloc_nums, aligned_size);
  if (actual_num == 1) {
    assert(start == end);
  } else {
    // start指向的内存块返回给线程使用，所以将剩下actual_num-1的内存块放入自由链表中备用
    free_lists_[hash_index].push(next_memptr(start), end, actual_num - 1);
  }
  return start;
}

  void ThreadCache::return_mem_to_cc(FreeList& free_list, std::size_t aligned_size){
    void* start = nullptr;
    void* end = nullptr;
    free_list.pop(start, end, aligned_size);
    CentralCache* CC = CentralCache::get_instance();
    CC->release_list_to_spans(start, aligned_size);

  }
void ThreadCache::expand_fetch_nums(std::size_t index) {
  assert(index <= FREE_LIST_NUM);
  if (fetch_mem_nums_[index] < 2) {
    ++fetch_mem_nums_[index];
  } else {
    fetch_mem_nums_[index] *= 1.5;
  }
}
} // namespace lmem