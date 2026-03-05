#include "thread_pool.hpp"
#include <chrono>
#include <iostream>

void task1() {
  std::cout << "task1 running\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void task2() {
  std::cout << "task2 running\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main(int argc, char *argv[]) {
  ThreadPool pool(4);

  for (int i = 0; i < 5; i++) {
    pool.submit(task1);
    pool.submit(task2);
  }
  return 0;
}