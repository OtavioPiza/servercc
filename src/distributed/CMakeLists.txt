add_library(distributed SHARED)

# Library sources and headers.
target_sources(
    distributed 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/distributed.cc
)

target_include_directories(
    distributed
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link dependencies.
target_link_libraries(
    distributed
    PRIVATE
        client
        connector
        server
)
