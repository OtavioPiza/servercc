add_library(server INTERFACE)
target_link_libraries(
    server
    INTERFACE
        absl::flat_hash_map
        types
)


add_library(tcp_server ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_server.cc)
target_include_directories(
    tcp_server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(
    tcp_server
    PRIVATE 
        libcc
        server
        types
)


add_library(udp_server ${CMAKE_CURRENT_SOURCE_DIR}/src/udp_server.cc)
target_include_directories(
    udp_server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(
    udp_server
    PRIVATE 
        absl::strings
        server
        types
)


add_library(servers INTERFACE)
target_link_libraries(
    servers
    INTERFACE
        server
        tcp_server
        udp_server
)
target_include_directories(
    servers
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

