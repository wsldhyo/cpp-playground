#ifndef PAGE_CACHE_HPP
#define PAGE_CACHE_HPP

#include "global_var.hpp"
#include "span_list.hpp"
#include <cstddef>
#include <mutex>
#include <unordered_map>
namespace lmem {


// 
// @brief 页缓存类
// PC中也有Span的哈希桶。但映射规则与CC和TC中的不一样。CC和TC中的哈希桶是依据内存块
// 大小进行映射的。而PC中的哈希桶是依据页数映射的，哈希索引为i的SpanList，其管理的
// 每个Span都维护i页内存。
// 同时Span中页不会划分为小块内存，划分工作由CC负责
// 
class PageCache{
public:
    static PageCache* get_instance();

    //
    // @brief 取出一个管理k页内存的Span
    //     内部实际通过get_raw_span_来获取span
    // @param [in] k Span管理内存页数,  k >=0 && k < MAX_PAGE_NUM
    // @return 管理k页的Span
    //
    Span* get_raw_span(std::size_t k);
    
    //
    // @brief CC归还Span给PC
    //     PageCache会尝试将span与相邻桶的Span合并为一个较大的Span
    //     因为所有的Span在分配给CC时，都是从最大的Span分裂的。合并
    //     为管理更多页数的Span，可以更好适应不同大小的内存需求
    // @param span 待放入PageCache哈希桶中的span
    // @return void
    //
    void return_span_to_pc(Span* span);
    Span* map_obj_to_span(void* obj)const;
private:
    //
    // @brief 取出一个管理k页内存的Span
    // @param [in] k Span管理内存页数,  k >=0 && k < MAX_PAGE_NUM
    // @return 管理k页的Span
    //
    Span* get_raw_span_(std::size_t k);
    PageCache() = default;
    PageCache(PageCache const&) = delete;
    PageCache& operator=(PageCache const&) = delete;
    
private:
    static PageCache* instance_;
    SpanList span_lists_[MAX_PAGE_NUM];
    mutable std::mutex page_mutex_;
    std::unordered_map<PageID_t, Span*>   pageID_span_map_;
};

}
#endif