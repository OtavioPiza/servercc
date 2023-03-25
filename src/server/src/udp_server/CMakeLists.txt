
add_library(udp_server SHARED)

target_sources(udp_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/udp_server.cc)
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
        libcc
        server_config
        server_types
        types
)
