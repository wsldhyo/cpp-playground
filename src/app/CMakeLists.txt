function(add_server_executable target source core)
    add_executable(${target}
        ${source}
    )

    target_link_libraries(${target}
        PRIVATE ${core}
        PRIVATE common
    )
endfunction()

add_server_executable(server
    server_main.cc
    server_core
)

add_server_executable(client
    client_main.cc
    client_core
)

add_server_executable(userver
    udp_serv_main.cc
    userver_core
)

add_server_executable(uclient
    udp_clnt_main.cc
    uclient_core
)

add_server_executable(mpst_server
    mpst_tcp_server_main.cc
    mpst_server_core
)

add_server_executable(spmt_server
    spmt_tcp_server_main.cc
    spmt_server_core
)
find_package(Threads REQUIRED)
target_link_libraries(spmt_server
    PRIVATE Threads::Threads
)

add_server_executable(epoll_tcp_server
    epoll_tcp_server_main.cc
    epoll_tcp_server_core
)