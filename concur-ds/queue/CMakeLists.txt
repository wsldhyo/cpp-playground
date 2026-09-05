add_library(queue_module INTERFACE)

#添加头文件路径
target_include_directories(queue_module INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)