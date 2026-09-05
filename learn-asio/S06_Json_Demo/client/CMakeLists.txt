find_package(jsoncpp REQUIRED)

# 添加可执行文件
add_executable(client src/main.cc)
target_link_libraries(client PRIVATE jsoncpp pthread)
target_compile_options(client PRIVATE -pthread -g)
set_target_properties(client PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)