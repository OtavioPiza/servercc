add_library(client INTERFACE)
target_link_libraries(
    client
    INTERFACE
        types
        absl::status
        absl::strings
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
        client
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
        client
)


add_library(clients INTERFACE)
target_link_libraries(
    clients
    INTERFACE
        client
        multicast_client
)
target_include_directories(
    clients
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
