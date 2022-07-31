add_library(
    response
    src/response.cc
    src/response.hh
)

target_include_directories(response PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
