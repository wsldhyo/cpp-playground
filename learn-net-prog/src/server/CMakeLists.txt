#通过接口库暴露各个server的hpp路径，其他源文件可以直接#include
add_library(server_common INTERFACE)

target_include_directories(server_common
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(server_common
    INTERFACE common
)

function(add_server_library target)
    add_library(${target} STATIC
        ${ARGN}
    )

    target_link_libraries(${target}
        PUBLIC server_common
    )
endfunction()

add_server_library(server_core
    tcp_server.cc
)

add_server_library(userver_core
    udp_server.cc
)

add_server_library(mpst_server_core
    mpst_tcp_server.cc
)

add_server_library(spmt_server_core
    spmt_tcp_server.cc
)

add_server_library(epoll_tcp_server_core
    epoll_tcp_channel.cc
    epoll_tcp_poller.cc
    epoll_tcp_eventloop.cc
    epoll_tcp_acceptor.cc
    epoll_tcp_connection.cc
)



add_server_library(iouring_tcp_server_core
    iouring_acceptor.cc
    iouring_connection.cc
    iouring_io_service.cc 
    iouring_io_backend.cc
    iouring_request.cc
)
target_link_libraries(iouring_tcp_server_core PRIVATE uring)