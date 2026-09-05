add_executable(server src/main.cc)
# 链接 proto_lib
target_link_libraries(server PRIVATE proto_lib)
# 头文件包含 proto 目录
target_include_directories(server PRIVATE ${CMAKE_BINARY_DIR})

set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#生成调试信息
target_compile_options(server PRIVATE -g)