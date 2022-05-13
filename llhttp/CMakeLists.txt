add_library(
    llhttp
    src/llhttp.c
    src/llhttp.h
)

target_include_directories(
    llhttp
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)