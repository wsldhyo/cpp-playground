#ifndef FREE_LIST_HPP
#define FREE_LIST_HPP
#include <cstddef>
namespace lmem {

//
// @brief 获取下一内存块的指针
//     当回收空闲空间时，
//     将空闲内存块前sizeof(void*)个字节用于存储下一内存块的地址，
//     不使用额外空间存储下一内存块的地址。函数返回的就是前sizeof(void*)字节构成的指针。
//     由于返回的是引用，让返回的指针指向另一内存，也将直接影响到内存块。
// @param [in] obj 指向内存块的指针
// @return void* 下一内存块的地址的引用
//
void *&next_memptr(void *obj);

//
// @brief 管理小块内存的自由链表类
//
class FreeList {
public:
  FreeList() = default;
  //
  // @brief 将obj指针头插到链表中
  // @description 用于回收内存块，作为空闲内存块备用
  // @param [in] obj 指向待回收内存的指针
  // @return void
  //
  void push(void *obj);

  void push(void *start, void *end, std::size_t nums);

  //
  // @brief 头删链表节点
  //    用于回收内存块，作为空闲内存块备用
  // @param void
  // @return 分配的空闲内存块的指针
  //
  void* pop();

  void pop(void*&start, void*end, std::size_t num);

  std::size_t size()const;

  bool empty() const;

private:
  void *list_head_ = nullptr; // 自由链表头指针
  std::size_t size_ = 0;
};

} // namespace lmem
#endif