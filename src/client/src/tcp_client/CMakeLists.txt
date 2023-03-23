add_library(tcp_client SHARED)

target_sources(tcp_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_client.cc)
target_include_directories(
    tcp_client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    tcp_client
    PRIVATE 
        client_types
        libcc
)
