add_library(server_types INTERFACE)

target_include_directories(
    server_types
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
