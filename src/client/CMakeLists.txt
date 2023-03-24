add_library(client INTERFACE)

target_include_directories(client INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    client
    INTERFACE
        client_types
        tcp_client
        multicast_client
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/client_types client_types)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_client tcp_client)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/multicast_client multicast_client)
