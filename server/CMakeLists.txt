add_library(
    server
    src/server.cc
    src/server.hh
)

target_include_directories(server PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_subdirectory("../router" router)
target_link_libraries(server router)

add_subdirectory("../util/logger" logger)
target_link_libraries(server logger)