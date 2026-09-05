cmake_minimum_required(VERSION 3.28)
project(echo_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
# 编译数据库， 给clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#输出文件目录 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message(STATUS "================ Compiler Info ================")
message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CXX Path    : ${CMAKE_CXX_COMPILER}")
message(STATUS "===============================================")

add_subdirectory(src)