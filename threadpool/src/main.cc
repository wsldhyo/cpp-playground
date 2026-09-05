#include "thread_pool.hpp"
#include <chrono>
#include <iostream>

int add(int a, int b) {
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  return a + b;
}

void test_submit() {
  LThreadPool pool(4);
  // 多种形式的task
  auto f1 = pool.submit(add, 1, 2);
  auto f2 = pool.submit(
      [](int x) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        return x * x;
      },
      5);
  auto f3 = pool.submit([] { std::cout << "hello from thread\n"; });

  std::cout << "add result: " << f1.get() << std::endl;
  std::cout << "square result: " << f2.get() << std::endl;

  f3.get(); // void future
}

void test_local_task() {
  LThreadPool pool(2);
  auto f = pool.submit([&pool]() {
    auto tid = std::this_thread::get_id();
    std::cout << "Task A running in thread: " << tid << std::endl;

    // 在 worker 线程里提交任务
    auto f2 = pool.submit([]() {
      auto tid2 = std::this_thread::get_id();
      std::cout << "Task B running in thread: " << tid2 << std::endl;
    });

    std::cout << "Task A finished\n";
  });

  f.get();
}

int main(int argc, char *argv[]) {
  test_local_task();
  return 0;
}
