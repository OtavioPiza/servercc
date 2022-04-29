add_library(
    server
    src/server.cc
    src/server.hh
)

target_include_directories(server PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")