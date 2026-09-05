cmake_minimum_required(VERSION 3.20)
project(AsioApiStudy)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
add_executable(ApiStudy src/main.cc
                src/class_and_func.cc)

target_include_directories(ApiStudy PRIVATE include)
