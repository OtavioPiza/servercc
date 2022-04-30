add_library(
    logger
    src/logger.cc
    src/logger.hh
)

target_include_directories(logger PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")