add_library(common STATIC
    common.cc
)

#暴露头文件路径
target_include_directories(common
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})