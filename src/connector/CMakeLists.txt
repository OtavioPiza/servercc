add_library(connector SHARED)

target_sources(connector PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/connector.cc)
target_include_directories(
    connector
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    connector
    PRIVATE 
        libcc
        client
)
