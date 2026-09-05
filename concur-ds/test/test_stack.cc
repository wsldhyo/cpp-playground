#include "test_stack.hpp"
#include <vector>
#include <thread>
#include <iostream>
// -------------------- 单独 push 测试 --------------------
void test_push(MtxStack<int>& stack, int thread_id, int num_ops) {
    for (int i = 0; i < num_ops; ++i) {
        stack.push(i + thread_id * 1000);
    }
}

// -------------------- 单独 pop 测试 --------------------
void test_pop(MtxStack<int>& stack, std::atomic<int>& pop_count) {
    int val;
    while (stack.pop(val)) {
        pop_count.fetch_add(1, std::memory_order_relaxed);
    }
}

// -------------------- 并发测试函数 --------------------
bool test_mtx_stack_concurrency(int num_threads, int num_ops) {
    MtxStack<int> stack;

    // 多线程 push
    std::vector<std::thread> push_threads;
    for (int t = 0; t < num_threads; ++t) {
        push_threads.emplace_back(test_push, std::ref(stack), t, num_ops);
    }
    for (auto& th : push_threads) th.join();

    // 多线程 pop
    std::vector<std::thread> pop_threads;
    std::atomic<int> pop_count{0};

    for (int t = 0; t < num_threads; ++t) {
        pop_threads.emplace_back(test_pop, std::ref(stack), std::ref(pop_count));
    }
    for (auto& th : pop_threads) th.join();

    // 检查结果
    bool success = stack.empty() && (pop_count == num_threads * num_ops);
    if (!success) {
        std::cerr << "Test failed! size = " << stack.size()
                  << ", popped = " << pop_count.load() << "\n";
    } else {
        std::cout << "Test sucess! size = " << stack.size()
                  << ", total popped = " << pop_count.load() << "\n";
    }
    return success;
}