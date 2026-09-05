#ifndef SYS_ALLOC_HPP
#define SYS_ALLOC_HPP

#include <cstddef>
namespace  lmem {
void* system_allocate(std::size_t page_num);
}


#endif