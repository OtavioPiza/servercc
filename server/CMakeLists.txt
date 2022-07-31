add_library(
    server
    src/server.cc
    src/server.hh
)

target_include_directories(server PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(server llhttp)

add_subdirectory("router" router)
target_link_libraries(server router)

add_subdirectory("../util/logger" logger)
target_link_libraries(server logger)

include(FetchContent)

FetchContent_Declare(llhttp
  URL "https://github.com/nodejs/llhttp/archive/refs/tags/release/v6.0.6.tar.gz")  # Using version 6.0.5

FetchContent_MakeAvailable(llhttp)