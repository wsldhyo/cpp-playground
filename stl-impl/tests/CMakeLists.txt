# 创建测试可执行文件
add_executable(test_unique_ptr test_unique_ptr.cc)

target_link_libraries(test_unique_ptr PRIVATE unique_ptr_intf)

# 添加测试用例，让 CTest 可以运行它: 通过 ctest 或 make test 运行
add_test(NAME test_unique_ptr COMMAND test_unique_ptr)