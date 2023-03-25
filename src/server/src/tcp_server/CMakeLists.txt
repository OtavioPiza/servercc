add_library(tcp_server SHARED)

target_sources(tcp_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_server.cc)
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
        server_config
        server_types
        types
)

