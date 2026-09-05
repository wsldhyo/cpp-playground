cmake_minimum_required(VERSION 3.20)
project(lmem VERSION 1.0.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

set(SRC_PATH "src")
set(SRC_FILES 
              free_list.cc
              size_class.cc
              thread_cache.cc
              span_list.cc
              central_cache.cc
              page_cache.cc
              sys_alloc.cc 
              concurrent_allocate.cc
)

#生成compile_commands.json， 给clangd使用。也可以在.clangd中配置
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)  

foreach(src_file IN LISTS SRC_FILES)
    list(APPEND FULL_SRC_FILES "${SRC_PATH}/${src_file}")
endforeach()
add_executable(main include/global_var.hpp ${FULL_SRC_FILES} test/main.cc)

target_include_directories(main PRIVATE include)

target_compile_options(main PRIVATE -pthread -g)

target_link_libraries(main PRIVATE -pthread)