
find_package(jsoncpp REQUIRED)

# 添加可执行文件
add_executable(server   src/main.cc 
                        src/io_context_pool.cc 
                        src/logic_system.cc 
                        src/server.cc 
                        src/session.cc 
                        ../common/msg_node.cc)

target_link_libraries(server PRIVATE jsoncpp pthread)
target_compile_options(server PRIVATE -pthread -g)
set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)