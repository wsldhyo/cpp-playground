#ifndef THREAD_CACHE_HPP
#define THREAD_CACHE_HPP

#include <cstddef>

#include "free_list.hpp"
#include "global_var.hpp"
namespace lmem {
 //
 // @brief 线程缓存类 
 //
 // ThreadCache（下称TC）管理每个线程的内存申请，有几个线程，就创建几个TC。
 //  
 // TC维护了不同内存块的自由链表，当线程向TC申请size大小的空间时，TC会从维护
 // 该size大小内存块的自由链表中取出内存块。如果所需内存块的自由链表没有空闲内
 // 存块时，TC会向CentralCache（下称CC）申请空间。
 // 
 // TC向CC申请size大小的内存块时，申请数量会比前次申请要多一些。多余的内存块放入
 // 自由链表中备用，降低向CC申请的频率。这是因为只有一个CC对象，多个线程申请时相同
 // size的内存块时，CC中需要加锁。所以要降低频次，减少加解锁次数
 //
 
class ThreadCache {
public:
  //
  // @brief 向TC申请空间
  //     TC会对size进行内存对齐，因此返回的内存块的大小通常比size要大
  // @param [in] size 申请内存的大小
  // @return 申请到的内存块首地址
  //
  void *allocate(std::size_t size);

  // 
  // @brief 向线程归还空间
  // @param [in] obj 待回收内存块的首地址
  // @param [in] size 待回收内存块的大小
  // @return void
  // 
  void deallocate(void* obj, std::size_t size);

  //
  // @brief 向CC申请空间
  //     当TC中空间不足时，TC需要向CC申请相应大小的内存备用。为了避免频繁向CC申请，
  //     TC每次都比前一次多申请内存，类似std::vector的扩容策略
  // @param [in] hash_index 申请内存块对应的哈希桶索引
  // @param [in] aligned_size 对齐后的内存块大小
  // @preturn CC返回的内存块首地址
  //
  void *fetch_mem_from_cc(std::size_t hash_index, std::size_t aligned_size);
  
  //
  // @brief 向CC归还内存块
  //     当某个自由链表中的空闲内存块过多时，TC就将这一部分内存块归还给CC，
  //     以让其他线程使用 
  // @param [in] free_list 管理待归还内存块的自由链表
  // @param [in] aligned_size 待归还内存块的大小
  // @return void
  //
  void return_mem_to_cc(FreeList& free_list, std::size_t aligned_size);

  //
  // 
  //
  void expand_fetch_nums(std::size_t index);

private:
  FreeList free_lists_[FREE_LIST_NUM];
  std::size_t fetch_mem_nums_[FREE_LIST_NUM] = {0};
};

inline thread_local ThreadCache *g_thread_cache = nullptr;
} // namespace lmem

#endif