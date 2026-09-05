#ifndef GLOBAL_VAR_HPP
#define GLOBAL_VAR_HPP

#include <cstddef>

namespace lmem {
// TC中哈希桶数量
inline static const std::size_t FREE_LIST_NUM = 208;

// TC中允许单次申请的最大内存块大小
inline static const std::size_t MAX_TC_BLOCK_SIZE = 256 * 1024;

inline static const std::size_t MAX_PAGE_NUM = 129;

inline static const std::size_t PAGE_SHIFT = 12;
} // namespace lmem

#endif