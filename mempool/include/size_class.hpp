#ifndef SIZE_CLASS_HPP
#define SIZE_CLASS_HPP

#include <cstddef>
namespace lmem {
class SizeClass {
public:
  //
  // @brief 进行内存对齐
  //     根据size的大小，确定预定义的对齐值，将size扩大为对齐值的倍数
  // @param [in] size 原始的内存块大小
  // @return 对齐后的内存块大小
  //
  static std::size_t round_up(std::size_t size);

  //
  // @brief 获取存储相应大小内存块的哈希桶索引
  // @param [in] size 内存块大小
  // @return 该大小内存块对应的哈希桶索引
  //
  static std::size_t get_hash_index(std::size_t size);

  //
  // @brief 确定内存块申请数量
  //     TC向CC申请内存时，CC的哈希桶需要加锁。所以需要降低TC向CC
  //     申请内存的频次。因此TC每次申请时，CC会返回多块大小符合要求的
  //     内存块。该函数用于确定TC一次申请多少块内存
  // @param [in] size 内存块大小
  // @return 该大小内存块对应的哈希桶索引
  //
  static std::size_t count_block_num(std::size_t size);

  static std::size_t count_page_num(std::size_t size);

private:
  //
  // @brief 实际执行内存对齐的函数
  // @param [in] size 原始的内存块大小
  // @param [in] align_val 对齐值
  // @return 对齐后的内存块大小
  //
  static std::size_t round_up_(std::size_t size, std::size_t align_val);

  //
  // @brief 实际执行获取哈希桶索引的函数
  // @param [in] size 内存块大小
  // @param [in] align_val
  // 对齐值的二进制位数。如对齐值是8，则二进制位数是3，因为1 << 3 == 8
  // @return 对应哈希桶索引
  //
  static std::size_t get_hash_index_(std::size_t size, std::size_t align_shift);
};
} // namespace lmem

#endif