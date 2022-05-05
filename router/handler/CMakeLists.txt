add_library(
    handler
    src/handler.cc
    src/handler.hh
)


target_include_directories(handler PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")