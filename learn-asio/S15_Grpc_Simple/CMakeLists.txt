cmake_minimum_required(VERSION 3.28)
project(grpc_demo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 查找 Protobuf 和 gRPC
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# 指定 .proto 文件
set(PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/proto/demo.proto)

# 生成 Protobuf 和 gRPC 代码
protobuf_generate(
    LANGUAGE cpp
    GENERATE_EXTENSIONS .pb.h .pb.cc
    OUT_VAR PROTO_SRCS
    PROTOS ${PROTO_FILES}
)
protobuf_generate(
    LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
    OUT_VAR GRPC_SRCS
    PROTOS ${PROTO_FILES}
)

# 创建 proto_lib 静态库
add_library(proto_lib STATIC ${PROTO_SRCS} ${GRPC_SRCS})
target_link_libraries(proto_lib PUBLIC 
    protobuf::libprotobuf 
    gRPC::grpc++ 
    gRPC::grpc
    utf8_validity  # 需要链接utf8_range的utf8_validity库
)
target_include_directories(proto_lib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(client)
add_subdirectory(server)