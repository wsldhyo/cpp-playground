#include "concurrent_allocate.hpp"
#include "page_cache.hpp"
#include "thread_cache.hpp"
namespace lmem {

void *concurrent_allocate(std::size_t size) {
  if (g_thread_cache == nullptr) {
    g_thread_cache = new ThreadCache;
  }
  return g_thread_cache->allocate(size);
}

void concurrent_deallocate(void *obj) {
  if(obj == nullptr)
  {
    return;
  }
  Span* span = PageCache::get_instance()->map_obj_to_span(obj);
  g_thread_cache->deallocate(obj, span->block_size);
}
} // namespace lmem