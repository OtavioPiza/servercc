add_library(client INTERFACE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/client_types client_types)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/udp_client upd_client)

target_include_directories(client INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    client
    INTERFACE
        client_types
        udp_client
)
