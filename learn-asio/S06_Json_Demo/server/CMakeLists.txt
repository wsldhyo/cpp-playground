find_package(jsoncpp REQUIRED)
# 添加可执行文件
add_executable(server src/main.cc src/session.cc src/server.cc)
target_link_libraries(server  PRIVATE jsoncpp pthread)
target_compile_options(server PRIVATE -pthread -g)
set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)