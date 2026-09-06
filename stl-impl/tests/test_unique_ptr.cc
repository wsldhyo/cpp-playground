// test_unique_ptr.cpp
#include "../src/smart_ptr/unique_ptr.hpp" // 根据实际路径调整
#include <cassert>
#include <iostream>
#include <utility> // std::move

// 辅助宏：输出测试通过信息
#define TEST_PASS(msg) std::cout << "[PASS] " << msg << std::endl

// 用于测试带有析构函数的对象，验证资源是否被正确释放
struct TestObject {
  static int alive_count;
  int value;

  explicit TestObject(int v = 0) : value(v) { ++alive_count; }
  ~TestObject() { --alive_count; }
};
int TestObject::alive_count = 0;

// 测试函数声明
void test_default_constructor();
void test_pointer_constructor();
void test_nullptr_constructor();
void test_move_constructor();
void test_move_assignment();
void test_self_move_assignment();
void test_reset();
void test_release();
void test_swap();
void test_comparison_operators();
void test_dereference_operators();
void test_bool_conversion();
void test_resource_management();
void test_deleter();

int main() {
  test_default_constructor();
  test_pointer_constructor();
  test_nullptr_constructor();
  test_move_constructor();
  test_move_assignment();
  test_self_move_assignment();
  test_reset();
  test_release();
  test_swap();
  test_comparison_operators();
  test_dereference_operators();
  test_bool_conversion();
  test_resource_management();
  test_deleter();
  std::cout << "\nAll tests passed successfully!\n";
  return 0;
}

// 1. 默认构造
void test_default_constructor() {
  using scratch::UniquePtr;
  UniquePtr<int> p;
  assert(p.get() == nullptr);
  assert(!p);
  TEST_PASS("Default constructor");
}

// 2. 从原始指针构造
void test_pointer_constructor() {
  using scratch::UniquePtr;
  int *raw = new int(42);
  UniquePtr<int> p(raw);
  assert(p.get() == raw);
  assert(*p == 42);
  TEST_PASS("Pointer constructor");
}

// 3. 从 nullptr 构造（隐式转换）
void test_nullptr_constructor() {
  using scratch::UniquePtr;
  UniquePtr<int> p = nullptr;
  assert(p.get() == nullptr);
  assert(!p);
  TEST_PASS("nullptr constructor");
}

// 4. 移动构造
void test_move_constructor() {
  using scratch::UniquePtr;
  UniquePtr<int> p1(new int(10));
  UniquePtr<int> p2(std::move(p1));
  assert(p1.get() == nullptr);
  assert(p2.get() != nullptr);
  assert(*p2 == 10);
  TEST_PASS("Move constructor");
}

// 5. 移动赋值
void test_move_assignment() {
  using scratch::UniquePtr;
  UniquePtr<int> p1(new int(20));
  UniquePtr<int> p2(new int(30));
  p2 = std::move(p1);
  assert(p1.get() == nullptr);
  assert(p2.get() != nullptr);
  assert(*p2 == 20);
  TEST_PASS("Move assignment");
}

// 6. 自移动赋值
void test_self_move_assignment() {
  using scratch::UniquePtr;
  UniquePtr<int> p(new int(40));
  UniquePtr<int> &ref = p;
  p = std::move(ref); // 自赋值
  assert(p.get() != nullptr);
  assert(*p == 40);
  TEST_PASS("Self move assignment");
}

// 7. reset
void test_reset() {
  using scratch::UniquePtr;
  UniquePtr<int> p(new int(50));
  p.reset();
  assert(p.get() == nullptr);

  p.reset(new int(60));
  assert(p.get() != nullptr);
  assert(*p == 60);

  int *raw = new int(70);
  p.reset(raw);
  assert(p.get() == raw);
  assert(*p == 70);
  TEST_PASS("reset");
}

// 8. release
void test_release() {
  using scratch::UniquePtr;
  UniquePtr<int> p(new int(80));
  int *raw = p.release();
  assert(p.get() == nullptr);
  assert(raw != nullptr);
  assert(*raw == 80);
  delete raw; // 手动释放
  TEST_PASS("release");
}

// 9. swap
void test_swap() {
  using scratch::UniquePtr;
  UniquePtr<int> p1(new int(90));
  UniquePtr<int> p2(new int(100));
  p1.swap(p2);
  assert(*p1 == 100);
  assert(*p2 == 90);
  TEST_PASS("swap");
}

// 10. 比较运算符
void test_comparison_operators() {
  using scratch::UniquePtr;
  UniquePtr<int> p1(new int(110));
  UniquePtr<int> p2(new int(120));
  UniquePtr<int> p3;

  // 与 nullptr 比较
  assert(p1 != nullptr);
  assert(nullptr != p1);
  assert(p3 == nullptr);
  assert(nullptr == p3);

  // 与其他 UniquePtr 比较
  assert(p1 == p1);
  assert(p1 != p2);
  assert(p1 != p3);
  assert(p3 == p3);
  TEST_PASS("Comparison operators");
}

// 11. 解引用运算符
void test_dereference_operators() {
  using scratch::UniquePtr;
  UniquePtr<std::string> p(new std::string("hello"));
  assert(p->size() == 5);
  assert(*p == "hello");
  (*p)[0] = 'H';
  assert(*p == "Hello");
  TEST_PASS("Dereference operators");
}

// 12. bool 转换
void test_bool_conversion() {
  using scratch::UniquePtr;
  UniquePtr<int> p;
  assert(!p);
  p.reset(new int(130));
  assert(p);
  // 注意：explicit operator bool 不能隐式转换为 int，但可以在条件中使用
  if (p) {
    // 有效
  } else {
    assert(false);
  }
  TEST_PASS("bool conversion");
}

// 13. 资源管理（使用带计数器的对象）
void test_resource_management() {
  using scratch::UniquePtr;
  assert(TestObject::alive_count == 0);
  {
    UniquePtr<TestObject> p1(new TestObject(1));
    assert(TestObject::alive_count == 1);
    {
      UniquePtr<TestObject> p2(new TestObject(2));
      assert(TestObject::alive_count == 2);
      p2.reset(new TestObject(3));
      assert(TestObject::alive_count == 2); // 旧的2已被释放，新的3存活
    }
    assert(TestObject::alive_count == 1); // p2 已销毁，释放了对象3
  }
  assert(TestObject::alive_count == 0); // p1 已销毁
  TEST_PASS("Resource management (destructor invoked correctly)");
}

// 全局计数器，用于验证删除器调用
static int delete_count = 0;

// 自定义删除器：记录删除次数
struct CountingDeleter {
  void operator()(int *p) const {
    ++delete_count;
    delete p;
  }
};

// 带状态的自定义删除器
struct StatefulDeleter {
  int id;
  explicit StatefulDeleter(int i) : id(i) {}
  void operator()(int *p) const {
    std::cout << "Deleting from stateful deleter id=" << id << "\n";
    delete p;
  }
};

// 测试函数
void test_deleter() {
  using scratch::UniquePtr;
  std::cout << "=== Testing UniquePtr deleter ===" << std::endl;
  // 1. 默认删除器
  {
    UniquePtr<int> p(new int(42));
    assert(p.get() != nullptr);
    // 离开作用域时自动 delete
  }

  // 2. 自定义删除器（无状态）
  {
    delete_count = 0;
    {
      UniquePtr<int, CountingDeleter> p(new int(1));
      assert(delete_count == 0);
    }
    assert(delete_count == 1); // 删除器被调用一次
  }

  // 3. 带状态删除器
  {
    StatefulDeleter d(100);
    UniquePtr<int, StatefulDeleter> p(new int(2), d);
    // 离开作用域时输出 "Deleting from stateful deleter id=100"
  }

  // 4. 移动构造：删除器被移动，源指针置空
  {
    delete_count = 0;
    {
      UniquePtr<int, CountingDeleter> p1(new int(3));
      UniquePtr<int, CountingDeleter> p2(std::move(p1));
      assert(p1.get() == nullptr);
      assert(p2.get() != nullptr);
      assert(delete_count == 0);
    }
    assert(delete_count == 1); // p2 析构释放资源
  }

  // 5. 移动赋值：旧资源用旧删除器释放，新删除器接管
  {
    delete_count = 0;
    UniquePtr<int, CountingDeleter> p1(new int(4));
    {
      UniquePtr<int, CountingDeleter> p2(new int(5));
      p1 = std::move(p2);
      assert(p2.get() == nullptr);
      assert(delete_count == 1); // p1 原来的 int(4) 被删除
    }
    // p2 已经为空，不会触发删除
    assert(delete_count == 1);
  } // p1 析构，删除 int(5)，delete_count 变为 2

  // 验证最终删除次数
  assert(delete_count == 2);

  // 6. reset 使用旧删除器释放旧指针
  {
    delete_count = 0;
    UniquePtr<int, CountingDeleter> p(new int(6));
    p.reset(new int(7)); // 释放 int(6)，delete_count 变为 1
    assert(delete_count == 1);
    p.reset(); // 释放 int(7)，delete_count 变为 2
    assert(delete_count == 2);
    assert(p.get() == nullptr);
  }

  // 7. nullptr 赋值
  {
    delete_count = 0;
    UniquePtr<int, CountingDeleter> p(new int(8));
    p = nullptr; // 相当于 reset()，释放 int(8)
    assert(delete_count == 1);
    assert(p.get() == nullptr);
  }

  // 8. 获取删除器
  {
    StatefulDeleter d(7);
    UniquePtr<int, StatefulDeleter> p(new int(9), d);
    auto &del = p.get_deleter();
    assert(del.id == 7);
  }
}