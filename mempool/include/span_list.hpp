#ifndef SPAN_LIST_HPP
#define SPAN_LIST_HPP
#include <cstddef>
#include <mutex>
namespace lmem {

using PageID_t = std::size_t;

//
// @brief 管理以页为单位的大内存
// 
// 
struct Span {
  // Span管理的内存页数
  // 每个物理内存页通常是4KB，对于小内存如8B，那么一个内存页就可以满足
  // 对于大内存如128KB，那么Span需要32个内存页
  std::size_t page_num = 0;     

// Span管理的首个内存页页号(页内存首地址) 
// pageID是首地址除以页大小计算得到的， 一个Span管理的页范围是[pageID, pageID +page_num)
  PageID_t pageID = 0;         
  
  // 方便后面实现Span的双向链表
  Span *prev = nullptr;         
  Span *next = nullptr;
  
  // 页内存需要拆分为不同大小的内存块，以便TC取用
  void *free_list = nullptr;

  std::size_t use_count = 0;
  bool used_by_cc = false;
  std::size_t block_size;
};

// 
// @brief Span的双向循环链表
//
class SpanList {
public:
  
  // 简单的迭代器
  struct iterator {
  public:
    iterator(Span* span = nullptr);
    Span *&operator++();
    Span *operator++(int);
    Span *&operator--();
    Span *operator--(int);
    Span *&operator*();
    Span *operator->();
    bool operator!=(iterator const &other) const;
    bool operator==(iterator const &other) const;

  private:
    Span *span_;
  };

  SpanList();
  ~SpanList();
  iterator begin();
  iterator end();
  void insert(Span *where, Span *data);
  void push_front(Span *span);
  Span* pop_front();
  void erase(Span *where);
  void lock() const;
  void unlock() const;
  bool empty() const;

  iterator operator++(int);

private:
  void insert_(Span *new_data, Span *prev, Span *next);
  void erase_(Span *prev, Span *next);

private:
  Span *head_;
  mutable std::mutex
      mutex_; // CC中的哈希桶锁，会有多个线程申请相同大小内存，访问同一哈希桶。要加锁
              // TODO换成自旋锁，开销更小
};
} // namespace lmem
#endif