add_library(udp_client SHARED)

target_sources(udp_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/udp_client.cc)
target_include_directories(
    udp_client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    udp_client
    PRIVATE 
        client_types
        utils
)
