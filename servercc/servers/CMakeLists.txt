add_library(server INTERFACE)
target_link_libraries(
    server
    INTERFACE
        absl::flat_hash_map
        types
)


add_library(tcp_request ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_request.cc)
target_include_directories(
    tcp_request
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(
    tcp_request
    PRIVATE
        absl::log
        absl::status
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
        absl::flat_hash_map
        absl::log
        absl::status
        absl::strings
        server
        tcp_request
        types
)


add_library(udp_request ${CMAKE_CURRENT_SOURCE_DIR}/src/udp_request.cc)
target_include_directories(
    udp_request
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(
    udp_request
    PRIVATE
        absl::status
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
        absl::flat_hash_map
        absl::log
        absl::status
        absl::strings
        server
        types
        udp_request
)


add_library(servers INTERFACE)
target_include_directories(
    servers
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    servers
    INTERFACE
        server
        tcp_server
        udp_server
        tcp_request
        udp_request
)
