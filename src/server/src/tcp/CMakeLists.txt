add_library(tcp SHARED)

target_sources(tcp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp.cc)
target_include_directories(
    tcp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    tcp
    PRIVATE 
        config
        libcc
        server
)

