add_library(server_core STATIC
    tcp_server.cc
)
add_library(userver_core STATIC
    udp_server.cc
)


add_library(mpst_server_core STATIC
    mpst_tcp_server.cc
)

add_library(spmt_server_core STATIC
    spmt_tcp_server.cc
)
#把 src/server 暴露给所有链接 server_core 的目标
#其他源文件可直接#include"tcp_server.hpp"， 无需#include “src/server/tcp_server.hpp”
target_include_directories(server_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(server_core 
    PRIVATE common
)

target_include_directories(userver_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(userver_core 
    PRIVATE common
)
target_include_directories(mpst_server_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(mpst_server_core 
    PRIVATE common
)

target_include_directories(spmt_server_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(spmt_server_core 
    PRIVATE common
)
