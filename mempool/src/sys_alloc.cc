#include "sys_alloc.hpp"
#include <cstddef>
#include <new>

#define LINUX
#ifdef WIN32
#include <windows.h>
#elif defined(LINUX)
#include <sys/mman.h>
#include <unistd.h>
#endif


namespace lmem {
void *system_allocate(std::size_t page_nums) {
  void *ptr = nullptr;
#ifdef WIN32
  ptr = VirtualAlloc(0, page_nums << PAGE_SHIFT, MEM_COMMIT | MEM_RESERVE,
                     PAGE_READWRITE);
#elif defined(LINUX)
  // 获取系统页面大小
  std::size_t page_size = sysconf(_SC_PAGESIZE);
  // 使用 mmap 进行内存分配
  ptr = mmap(nullptr, page_nums * page_size, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ptr == MAP_FAILED) {
    ptr = nullptr; // mmap失败，指针置为nullptr
  }
#endif
  if (ptr == nullptr) {
    throw std::bad_alloc();
  }
  return ptr;
}
} // namespace lmem