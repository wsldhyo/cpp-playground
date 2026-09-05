cmake_minimum_required(VERSION 3.20)
project(echo_server)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
add_subdirectory(client)
add_subdirectory(server)

