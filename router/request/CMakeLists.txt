add_library(
    request
    src/request.cc
    src/request.hh
)

add_executable(
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    request
)