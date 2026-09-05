add_library(client_core STATIC
    tcp_client.cc
)

target_include_directories(client_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(client_core
    PRIVATE common
)

add_library(uclient_core STATIC
    udp_clnt.cc
)

target_include_directories(uclient_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(uclient_core
    PRIVATE common
)