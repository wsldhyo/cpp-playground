# 添加 Protobuf 搜索路径
list(APPEND CMAKE_PREFIX_PATH "/usr/local/protobuf")

# 查找 Protobuf 包
find_package(Protobuf REQUIRED)

add_library(protobuf::gmock UNKNOWN IMPORTED)
# 设置一个空的 IMPORTED_LOCATION，这样链接时不会实际链接任何库
set_target_properties(protobuf::gmock PROPERTIES IMPORTED_LOCATION "")
find_package(absl REQUIRED)
# 打印输出 Protobuf 包含目录
message(STATUS "Protobuf include directories: ${Protobuf_INCLUDE_DIRS}")

# 添加包含目录
include_directories(${Protobuf_INCLUDE_DIRS})

# 生成 Protobuf 源文件
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS proto_msg/msg.proto)

# 添加可执行文件
add_executable(client src/main.cc ${PROTO_SRCS} ${PROTO_HDRS})

# 链接 Protobuf 库
target_link_libraries(client  PRIVATE ${Protobuf_LIBRARIES} absl::log_internal_check_op pthread)
set_target_properties(client PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_compile_options(client PRIVATE -pthread -g)