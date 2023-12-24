add_library(client INTERFACE)
target_link_libraries(
    client
    INTERFACE
        absl::status
        absl::strings
        types
)


add_library(clients INTERFACE)
target_link_libraries(
    clients
    INTERFACE
        client
        multicast_client
        tcp_client
)
target_include_directories(
    clients
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)


add_library(multicast_client ${CMAKE_CURRENT_SOURCE_DIR}/src/multicast_client.cc)
target_include_directories(
    multicast_client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(
    multicast_client
    PRIVATE 
        absl::status
        absl::strings
        client
        types
)


add_library(tcp_client ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_client.cc)
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
        absl::status
        absl::strings
        client
        types
)
