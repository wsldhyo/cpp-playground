add_library(test_stack INTERFACE)

target_sources(test_stack INTERFACE
    test_mtx_stack.cc
)

# 传递 include 路径
target_include_directories(test_stack INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)