
#include <iostream>
#include "concurrent_allocate.hpp"
int main(int argc, char *argv[]) {
  auto ptr1 = lmem::concurrent_allocate(15);
  auto ptr2 = lmem::concurrent_allocate(3);
  auto ptr3 = lmem::concurrent_allocate(82);
  auto ptr4 = lmem::concurrent_allocate(16);
  auto ptr5 = lmem::concurrent_allocate(278);

  lmem::concurrent_deallocate(ptr1);
  lmem::concurrent_deallocate(ptr2);
  lmem::concurrent_deallocate(ptr3);
  lmem::concurrent_deallocate(ptr4);
  lmem::concurrent_deallocate(ptr5);
  std::cout << "exit" << std::endl;
  return 0;
}