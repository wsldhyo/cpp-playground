#include "central_cache.hpp"
#include "free_list.hpp"
#include "global_var.hpp"
#include "page_cache.hpp"
#include "size_class.hpp"
#include <cassert>

namespace lmem {


CentralCache* CentralCache::instance_ = nullptr;

CentralCache *CentralCache::get_instance() {
  if (instance_ == nullptr) {
    instance_ = new CentralCache();
  }
  return instance_;
}

std::size_t CentralCache::fetch_batch_mems(void *&start, void *&end,
                                           std::size_t alloc_nums,
                                           std::size_t size) {
  auto hash_index = SizeClass::get_hash_index(size);
  auto &span_list = span_lists_[hash_index];
  span_list.lock();      // 可能有多个线程访问同一size的哈希桶，加锁保护
  auto span = get_one_span(span_list, size);
  assert(span);
  assert(span->free_list); // span管理的空间不能为空

  start = span->free_list;
  end = span->free_list;
  std::size_t actual_num = 1;
  // 将end后移alloc_nums块内存块，加上start指向的内存块数，就是期望的内存数
  for (std::size_t i = 0; i < alloc_nums - 1 && next_memptr(end) != nullptr;
       ++i) {
    end = next_memptr(end);
    ++actual_num;
  }

  next_memptr(end) = nullptr;       // 取出的空间不要再与Span中的空间关联
  span->free_list = next_memptr(end);   // 从Span中取出空间后，调整Span的自由链表
  span->use_count += actual_num;
  span_list.unlock();
  return actual_num;
}


  void CentralCache::release_list_to_spans(void* start, std::size_t size){
    std::size_t hash_index = SizeClass::get_hash_index(size);
    auto& span_list = span_lists_[hash_index];
    span_list.lock();
    void* next =nullptr;
    Span* span = nullptr;
    PageCache* PC = PageCache::get_instance();
    while(start)
    {
      next = next_memptr(start);
      span = PC->map_obj_to_span(start);
      // 将内存块归还给管理所在页的Span
      next_memptr(start) = span->free_list;
      span->free_list = start;
      --span->use_count;

      // Span的所有空间都已经归还，将当前Span归还给PC
      if (span->use_count == 0) {
        // 先将span从CC的桶中去掉
        span_list.erase(span);
        // 重置span
        span->free_list = nullptr;
        span->next = nullptr;
        span->prev = nullptr;
        span_list.unlock();     // 等待PC完成操作前，让其他线程可以操作当前桶
        PC->return_span_to_pc(span);
        span_list.lock();
      }
      start = next;
    }
    span_list.unlock();
  }

Span *CentralCache::get_one_span(SpanList &span_list, std::size_t size) {
  auto iter = span_list.begin();
  while (iter != span_list.end()) {
    if (iter->free_list) {
      return *iter;
    }
    ++iter;
  }
  span_list.unlock();
  std::size_t k = SizeClass::count_page_num(size);
  PageCache *page_cache = PageCache::get_instance();
  
  Span *new_span = page_cache->get_raw_span(k);
  new_span->used_by_cc = true;

  char *start = (char *)(new_span->pageID << PAGE_SHIFT);
  char *end = (char *)(start + (new_span->page_num << PAGE_SHIFT));

  // 切分new_span的空间，并将其串成链表
  new_span->block_size = size;
  new_span->free_list = start;
  void *tail = start;
  start += size;
  while (start < end) {
    next_memptr(tail) = start;
    start += size;
    tail = next_memptr(tail);
  }
  next_memptr(tail) = nullptr;
  span_list.lock();  // 写入Span时先加上锁  
  span_list.push_front(new_span);
  return new_span;
}
} // namespace lmem