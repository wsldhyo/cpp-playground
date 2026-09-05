#include "test_class.hpp"
#include <cstdio>
#include <utility>
TestClass::TestClass() {
  p = new int(1);
  fprintf(stdout, "%p:default ctor\n", this);
}

TestClass::~TestClass() {
  delete p;
  fprintf(stdout, "%p:dctor\n", this);
}

TestClass::TestClass(TestClass const &other) : p(new int(*other.p)) {
  fprintf(stdout, "%p:copy ctor\n", this);
}

TestClass &TestClass::operator=(TestClass const &other) {
  if (this != &other) {
    int *tmp = new int(*other.p);
    delete p;
    p = tmp;
  }
  fprintf(stdout, "%p:copy op=\n", this);
  return *this;
}

TestClass::TestClass(TestClass &&other) : p(std::exchange(other.p, nullptr)) {
  fprintf(stdout, "%p:move ctor\n", this);
}

TestClass &TestClass::operator=(TestClass &&other) {
  if (this != &other) {
    delete p;
    p = std::exchange(other.p, nullptr);
  }
  fprintf(stdout, "%p:move op=\n", this);
  return *this;
}