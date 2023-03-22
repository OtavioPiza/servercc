add_library(server_config INTERFACE)

target_include_directories(
    server_config
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
