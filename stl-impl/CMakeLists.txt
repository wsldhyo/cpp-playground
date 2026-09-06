cmake_minimum_required(VERSION 3.31)
project(cpp_from_scratch VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# clangd编译数据库
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 
# 可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) 

message(STATUS "================ Compiler Info ================")
message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CXX Path    : ${CMAKE_CXX_COMPILER}")
message(STATUS "OUTPUT_DIR  : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "===============================================")

add_subdirectory(src)
enable_testing()
add_subdirectory(tests)