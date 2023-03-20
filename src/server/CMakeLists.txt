add_library(server INTERFACE)

target_include_directories(server INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    server
    INTERFACE
        config
        tcp
        types
        udp
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/config config)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/tcp tcp)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/types types)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/udp udp)
