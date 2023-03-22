cmake_minimum_required(VERSION 3.22.1)

project(servercc)

set(CMAKE_CXX_STANDARD 20)

add_library(${PROJECT_NAME} INTERFACE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/server)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/client)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/libcc libcc)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    ${PROJECT_NAME}
    INTERFACE
        server
        client
)

if (${PROJECT_IS_TOP_LEVEL})
    enable_testing()
endif()
