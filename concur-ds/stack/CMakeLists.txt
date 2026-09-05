add_library(stack_module INTERFACE)

#传递当前目录头文件路径
target_include_directories(stack_module INTERFACE 
 ${CMAKE_CURRENT_SOURCE_DIR}
)