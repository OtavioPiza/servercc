add_library(multicast_client SHARED)

target_sources(multicast_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/multicast_client.cc)
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
        client_types
        libcc
)
