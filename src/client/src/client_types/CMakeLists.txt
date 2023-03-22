add_library(client_types INTERFACE)

target_include_directories(
    client_types
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
