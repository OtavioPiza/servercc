add_library(
    llhttp
    llhttp.c
    llhttp.h
)

target_include_directories(
    llhttp
    $CMAKE_CURRENT_SOURCE_DIR
)