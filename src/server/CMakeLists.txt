add_library(server INTERFACE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/server_config server_config)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/server_types server_types)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_server tcp_server)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/udp_server udp_server)

target_include_directories(server INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    server
    INTERFACE
        server_config
        server_types
        tcp_server
        udp_server
)
