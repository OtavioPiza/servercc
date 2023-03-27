cmake_minimum_required(VERSION 3.22.1)

project(servercc)

set(CMAKE_CXX_STANDARD 20)

# Create a library.
add_library(${PROJECT_NAME} INTERFACE)

# Set the include directory.
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add subdirectories.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/client)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/connector)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/distributed)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/server)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/types)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/libcc libcc)

# Link libraries.
target_link_libraries(
    ${PROJECT_NAME}
    INTERFACE
        client
        connector
        distributed
        server
        types
)

# Run enable testing if this is the top level project.
if (${PROJECT_IS_TOP_LEVEL})
    enable_testing()
endif()
