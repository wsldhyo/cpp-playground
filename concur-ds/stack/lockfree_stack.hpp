#ifndef LOCKFREE_STACK_HPP
#define LOCKFREE_STACK_HPP
#include <atomic>
template <typename T> class LockFreeStack {
  struct Node {
    T val;
    Node *next;

    template <typename U>
    explicit Node(U &&value) : val(std::forward<U>(value)), next(nullptr) {}
  };

public:
  LockFreeStack() noexcept
      : head_(nullptr), to_be_del_nodes_(nullptr), pop_thread_num_(0) {}

  // 类内部有有原子指针和回收协议状态（pop_thread_num_），禁止拷贝和移动
  // 不允许拷贝/赋值（否则需要复杂的深拷贝同步）
  LockFreeStack(const LockFreeStack &) = delete;
  LockFreeStack &operator=(const LockFreeStack &) = delete;

  // 可以允许移动（但实现较复杂，这里直接禁用更安全）
  LockFreeStack(LockFreeStack &&) = delete;
  LockFreeStack &operator=(LockFreeStack &&) = delete;

  ~LockFreeStack() {
    // 要求析构时没有其他线程在访问，clear非线程安全
    clear();
  }

public:
  template <typename U> void push(U &&val) {
    Node *const new_node{new Node(std::forward<U>(val))};

    new_node->next = head_.load(std::memory_order_relaxed);
    // 成功时用 release：发布 new_node 及其 val/next
    // 失败时 relaxed：只是重试，不需要同步
    while (!head_.compare_exchange_weak(new_node->next, new_node,
                                        std::memory_order_release,
                                        std::memory_order_relaxed)) {
    }
  }

  bool try_pop(T &val) {
    // 仅用于计数（参与回收协议），不承担数据同步， relaxed 足够
    pop_thread_num_.fetch_add(1, std::memory_order_relaxed);

    Node *old_head = head_.load(std::memory_order_relaxed);
    // 成功时 acquire：与 push 的 release 配对，读取到正确的 val
    // 失败时 relaxed：仅重试
    while (old_head != nullptr &&
           !head_.compare_exchange_weak(old_head, old_head->next,
                                        std::memory_order_acquire,
                                        std::memory_order_relaxed)) {
    }

    if (old_head == nullptr) {
      // 最后一个 pop 线程需要清理 pending list
      //  - acquire：如果这是最后一个线程，要看到其他线程挂入的节点
      if (pop_thread_num_.fetch_sub(1, std::memory_order_acquire) == 1) {
        // 可见性已经由if条件里 pop_thread_num_ 的 acquire语义
        // 保证，relaxed即可
        delete_nodes(
            to_be_del_nodes_.exchange(nullptr, std::memory_order_acquire));
      }
      return false;
    }

    val = std::move(old_head->val); // 要求移动操作不会抛出异常

    try_reclaim(old_head);
    return true;
  }

  bool empty() const noexcept {
    return head_.load(std::memory_order_relaxed) == nullptr;
  }

  // 非线程安全，仅用于析构或外部已停止并发
  void clear() noexcept {
    // 删除头节点
    delete_nodes(head_.exchange(nullptr, std::memory_order_relaxed));
    // 删除侯删链表节点
    delete_nodes(to_be_del_nodes_.exchange(nullptr, std::memory_order_acquire));
  }

private:
  // 尝试回收一个已经从栈中摘下的节点 old_head
  //
  // 协议核心：
  // 1. 若当前线程是“唯一活跃的 pop 线程”，可以直接删除：
  //      - 因为不存在其他线程仍可能访问 old_head
  //
  // 2. 若存在其他活跃 pop 线程：
  //      - 不能立即 delete
  //      - 必须将节点挂入 to_be_del_nodes_，延迟删除
  //
  // 生命周期同步关键：
  //   非最后线程：
  //     chain_pending_nodes(...)  (普通写)
  //     ↓
  //     pop_thread_num_.fetch_sub(..., release)
  //
  //   最后线程：
  //     pop_thread_num_.fetch_sub(..., acquire) == 1
  //     ↓
  //     delete_nodes(...)
  //
  //   上述 release → acquire 建立 happens-before：
  //         所有挂入 to_be_del_nodes_ 的节点，对最后线程可见
  //    以确保删除所有节点
  void try_reclaim(Node *old_head) {
    // 如果当前只有一个pop线程，则释放侯删链表的所有节点内存
    // “快路径判断”：是否可能是唯一线程， 不作为最终正确性依据，relaxed 足够
    if (pop_thread_num_.load(std::memory_order_relaxed) == 1) {
      // relaxed 即可：真正同步由下面的 fetch_sub 完成
      Node *nodes_to_del =
          to_be_del_nodes_.exchange(nullptr, std::memory_order_acquire);
      // 核查pop_thread_num_是否还是1, 以进行真正的删除操作
      //   acquire：看到其他线程 release 的链表写入，确认是否是最后一个线程
      if (pop_thread_num_.fetch_sub(1, std::memory_order_acquire) == 1) {
        delete_nodes(nodes_to_del);
      } else if (nodes_to_del) {
        // 不是最后一个线程，不能删除，把链表再挂回去
        chain_pending_nodes(nodes_to_del);
      }

      delete old_head;

    } else {
      // 非最后线程：把节点挂入待删除链表
      chain_pending_nodes(old_head, old_head);

      // release
      // 保证：
      //   “把节点挂入 to_be_del_nodes_” happens-before
      //   “其他线程通过 acquire 观察到计数变化并接管清理”
      pop_thread_num_.fetch_sub(1, std::memory_order_release);
    }
  }

  // 批量挂入多个节点（链表形式）
  void chain_pending_nodes(Node *nodes) {
    Node *last = nodes;
    while (Node *const next = last->next) {
      last = next;
    }
    chain_pending_nodes(nodes, last);
  }

  // 将一个（或一组）节点挂入待删除链表 to_be_del_nodes_
  void chain_pending_nodes(Node *first, Node *last) {

    last->next = to_be_del_nodes_.load(std::memory_order_relaxed);
    // 将first挂到last之后
    // 成功序，让其他线程看到完整的侯删链表（发布last->next）
    while (!to_be_del_nodes_.compare_exchange_weak(last->next, first,
                                                   std::memory_order_release,
                                                   std::memory_order_relaxed)) {
    }
  }

  // 删除节点内存
  void delete_nodes(Node *nodes) noexcept {
    while (nodes) {
      Node *next = nodes->next;
      delete nodes;
      nodes = next;
    }
  }

private:
  std::atomic<Node *> head_; // 通过其进行数据同步
  std::atomic<Node *> to_be_del_nodes_;
  std::atomic_uint32_t pop_thread_num_; // 通过其进行生命周期同步
};
#endif // LOCKFREE_STACK_HPP