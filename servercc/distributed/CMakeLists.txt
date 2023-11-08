add_library(distributed_server ${CMAKE_CURRENT_SOURCE_DIR}/src/distributed_server.cc)
target_include_directories(
    distributed_server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(
    distributed_server
    PRIVATE
        absl::log
        absl::flat_hash_map
        absl::flat_hash_set
        absl::status
        absl::strings
        clients
        connectors
        servers
    PUBLIC
        libcc
)


add_library(distributed INTERFACE)
target_link_libraries(
    distributed
    INTERFACE
        distributed_server
)
