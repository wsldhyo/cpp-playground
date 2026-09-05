#include "page_cache.hpp"
#include "global_var.hpp"
#include "span_list.hpp"
#include "sys_alloc.hpp"
#include <cassert>
#include <mutex>

namespace lmem {

PageCache* PageCache::instance_  = nullptr;
PageCache *PageCache::get_instance() {
  if (instance_ == nullptr) {
    instance_ = new PageCache();
  }
  return instance_;
}

Span *PageCache::get_raw_span(std::size_t k) {
  // 只有一个PageCache，加锁保证多线程安全
  std::lock_guard<std::mutex> gaurd(page_mutex_);
  return get_raw_span_(k);
}

Span *PageCache::get_raw_span_(std::size_t k) {
  assert(k > 0 && k < MAX_PAGE_NUM);

  // k号桶中有Span，直接取出
  if (!span_lists_[k].empty()) {
    Span *span = span_lists_[k].pop_front();
    // 记录分配出去的页与其地址的映射关系，方便回收
    for (int i = 0; i < span->page_num; ++i) {
      pageID_span_map_[span->pageID + i] = span;
    }
    return span;
  }

  // k号桶中没有Span，
  // 但后面i号桶中有Span，则将i号桶的Span拆分为k号Span和i-k号Span
  // 然后返回管理k页的Span，同时将i-k页的Span放入i-k号哈希桶中备用
  for (int i = k + 1; i < MAX_PAGE_NUM; ++i) {
    if (!span_lists_[i].empty()) {
      Span *i_span = span_lists_[i].pop_front();
      Span *k_span = new Span;

      // 将i页Span切分为k页Span和i-k页Span
      k_span->pageID = i_span->pageID;
      k_span->page_num = k;

      // pageID是首地址除以页大小计算出来的, 将前面k个页划分出去后
      // 剩下i-k个页的首地址就需要从原地址偏移k个页的大小
      // k页大小除以页大小就是k，所以i-k页的页号加上k即可
      i_span->pageID += k;
      i_span->page_num -= k;

      // 将切分后的i-k页Span放入对应的哈希桶中
      span_lists_[i_span->page_num].push_front(i_span);
      // 记录i-k页Span的信息，方便后续回收空间时，重新合并为i页Span
      pageID_span_map_[i_span->pageID] = i_span;
      pageID_span_map_[i_span->pageID + i_span->page_num - 1] = i_span;
      for (int i = 0; i < k_span->page_num; ++i) {
        pageID_span_map_[k_span->pageID + i] = k_span;
      }
      span_lists_[i_span->page_num].push_front(i_span);
      return k_span;
    }
  }
  // 后面的i号桶中也没有Span时，直接创建管理最大页数的Span，并直接
  // 向OS申请Span能持有的最大页数内存
  Span *big_span = new Span();
  void *ptr = system_allocate(MAX_PAGE_NUM - 1);
  big_span->pageID = ((PageID_t)ptr) >> PAGE_SHIFT;
  big_span->page_num = MAX_PAGE_NUM - 1;
  span_lists_[MAX_PAGE_NUM - 1].push_front(big_span);
  return get_raw_span_(k);
}

void PageCache::return_span_to_pc(Span *span) {
  std::lock_guard<std::mutex> gaurd(page_mutex_);
  // 因为pageID是根据页首地址除以页大小得到的。所以pageID相邻的页
  // 就是pageID-1和pageID+1，因为分配时在pageID_span_map记录页号和
  // 页首地址的关系，那么通过映射可以找到相邻页的首地址

  // 尝试和左边桶的Span合并
  while (true) {
    PageID_t left_ID = span->pageID - 1;
    auto res = pageID_span_map_.find(left_ID);
    if (res == pageID_span_map_.end()) {
      break;
    }
    Span *left_span = res->second;
    // pageID_span_map_只记录了页号和Span的关系，不保证Span是否挂在PC的哈希桶中
    if (left_span->used_by_cc) {
      break;  
    }
    if (left_span->page_num + span->page_num > MAX_PAGE_NUM - 1) {
      break;     // 合并后的Span不能超过一个Span允许管理的最大页数
    }
    
    // 合并，左边的span首地址成为连续页的首地址，进行相应属性修改
    span->pageID = left_span->pageID;
    span->page_num += left_span->page_num;
    span_lists_[left_span->page_num].erase(left_span);
    delete left_span;
  }

  // 尝试向右合并，逻辑与左合并类型
  while (true) {
    PageID_t right_ID = span->pageID + span->page_num;
    auto res = pageID_span_map_.find(right_ID);

    if (res == pageID_span_map_.end()) {
      break;
    }

    Span *right_span = res->second;
    if (right_span->used_by_cc) {
      break;
    }
    if (right_span->page_num + span->page_num > MAX_PAGE_NUM - 1) {
      break;
    }
    // 当前页就是合并后的连续页首地址，不需要修改pageID
    span->page_num += right_span->page_num;
    span_lists_[right_span->page_num].erase(right_span);
    delete right_span;
  }
  span_lists_[span->page_num].push_front(span);
  span->used_by_cc = false;
  pageID_span_map_[span->pageID] = span;
  pageID_span_map_[span->pageID + span->page_num] = span;
}

Span *PageCache::map_obj_to_span(void *obj) const {
  PageID_t pageID = ((PageID_t)obj) >> PAGE_SHIFT;
  auto res = pageID_span_map_.find(pageID);
  if (res != pageID_span_map_.end()) {
    return res->second;
  } else {
    assert(false);
    return nullptr;
  }
}
} // namespace lmem