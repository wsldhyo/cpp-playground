cmake_minimum_required(VERSION 3.28)
project(beast_http_server)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
find_package(jsoncpp REQUIRED)
add_subdirectory(server)

