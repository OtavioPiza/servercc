add_library(
    router
    src/router.cc
    src/router.hh
)

target_include_directories(router PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_subdirectory("./handler" handler)
target_link_libraries(router handler)
