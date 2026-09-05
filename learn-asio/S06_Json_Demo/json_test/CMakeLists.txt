cmake_minimum_required(VERSION 3.20)
project(async_echo_server)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
#注意编译jsoncpp所使用的C++版本，可能会出现ABI不兼容的情况
find_package(jsoncpp REQUIRED)
add_executable(main main.cc)
target_link_libraries(main PRIVATE jsoncpp)

