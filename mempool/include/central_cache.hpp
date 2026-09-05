#ifndef CENTRAL_CACHE_HPP
#define CENTRAL_CACHE_HPP

#include "global_var.hpp"
#include "span_list.hpp"
#include <cstddef>
namespace lmem {
class CentralCache {
public:
  static CentralCache *get_instance();

  //
  // @brief 提供多块空闲内存
  //
  // @param [out] start CC返回的内存首地址
  // @param [out] end CC返回的内存末地址
  // @param [in] apply_num 需要申请的内存块数量
  // @param [in] 需要申请的内存块大小
  // @return 实际申请的内存块数量
  // 
  std::size_t fetch_batch_mems(void *&start, void *&end, std::size_t apply_num,
                               std::size_t size);

  void release_list_to_spans(void* start, std::size_t size);

  // 
  // @brief 取出一个管理非空内存的Span 
  Span *get_one_span(SpanList &span_list, std::size_t size);



private:
  CentralCache() = default;
  CentralCache(CentralCache const &) = delete;
  CentralCache &operator=(CentralCache const &) = delete;

private:
  SpanList span_lists_[FREE_LIST_NUM]; // CC中的哈希结构，每个桶是一个SpanList，
                                       // 管理一个个Span
  static CentralCache *instance_;
};

} // namespace lmem

#endif