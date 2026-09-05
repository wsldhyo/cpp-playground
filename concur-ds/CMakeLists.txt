cmake_minimum_required(VERSION 3.31)
project(concur_cds  CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(stack)
add_subdirectory(queue)
add_subdirectory(test)

add_executable(main)
target_sources(main PRIVATE
test/main.cc
)

target_link_libraries(main PRIVATE test_stack stack_module queue_module)