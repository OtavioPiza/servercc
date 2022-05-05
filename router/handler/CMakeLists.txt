add_library(
    handler
    src/handler.cc
    src/handler.hh
)

add_subdirectory(../../request request)
target_link_libraries(handler request)

target_include_directories(handler PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")