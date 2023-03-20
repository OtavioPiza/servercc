
add_library(udp SHARED)

target_sources(udp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/udp.cc)
target_include_directories(
    udp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    udp
    PRIVATE 
        config
        libcc
        types
)

