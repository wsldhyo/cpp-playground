#ifndef TEST_MTX_STACK_HPP
#define TEST_MTX_STACK_HPP
#include "mtx_stack.hpp"
#include "lockfree_stack.hpp"
#include <atomic>
// -------------------- 单独 push 测试 --------------------
void test_push(MtxStack<int> &stack, int thread_id, int num_ops);

// -------------------- 单独 pop 测试 --------------------
void test_pop(MtxStack<int> &stack, std::atomic<int> &pop_count);

// -------------------- 整体并发测试函数 --------------------
bool test_mtx_stack_concurrency(int num_threads = 4, int num_ops = 1000);


void test_push_lockfree(LockFreeStack<int> stack, int thread_id, int num_ops);
#endif // TEST_MTX_STACK_HPP