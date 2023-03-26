add_library(client INTERFACE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/client_types client_types)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/multicast_client multicast_client)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_client tcp_client)

target_include_directories(client INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    client
    INTERFACE
        client_types
        multicast_client
        tcp_client
)
