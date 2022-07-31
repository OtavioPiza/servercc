add_library(
    request
    src/request.cc
    src/request.hh
)
target_include_directories(request PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")