#include "thread_pool.hpp"
#include <chrono>
#include <iostream>


int add(int a, int b)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return a + b;
}

int main()
{
    ThreadPool pool(4);
    // 多种形式的task
    auto f1 = pool.submit(add, 1, 2);
    auto f2 = pool.submit([](int x) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        return x * x;
    }, 5);
    auto f3 = pool.submit([] {
        std::cout << "hello from thread\n";
    });

    std::cout << "add result: " << f1.get() << std::endl;
    std::cout << "square result: " << f2.get() << std::endl;

    f3.get(); // void future
    return 0;
}