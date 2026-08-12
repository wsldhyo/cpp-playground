add_executable(server server_main.cc)
add_executable(client client_main.cc)
add_executable(userver udp_serv_main.cc)
add_executable(uclient udp_clnt_main.cc)
add_executable(mpst_server mpst_tcp_server_main.cc)
add_executable(spmt_server spmt_tcp_server_main.cc)
target_link_libraries(server
    PRIVATE server_core
    PRIVATE common
)
target_link_libraries(client
    PRIVATE client_core
    PRIVATE common
)
target_link_libraries(userver
    PRIVATE userver_core
    PRIVATE common
)
target_link_libraries(uclient
    PRIVATE uclient_core
    PRIVATE common
)
target_link_libraries(mpst_server
    PRIVATE mpst_server_core
    PRIVATE common
)
target_link_libraries(spmt_server
    PRIVATE spmt_server_core
    PRIVATE common
)