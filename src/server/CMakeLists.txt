add_library(server SHARED)

target_sources(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/server.cc)
target_include_directories(
    server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(server PRIVATE libcc)
