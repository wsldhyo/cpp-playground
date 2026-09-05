#ifndef TEST_CLASS_HPP
#define TEST_CLASS_HPP
class TestClass {
public:
    TestClass();
    ~TestClass();
    TestClass(TestClass const& other);
    TestClass& operator=(TestClass const& other);
    TestClass(TestClass && other);
    TestClass& operator=(TestClass && other);
private:
    int* p {nullptr};
};
#endif // TEST_CLASS_HPP