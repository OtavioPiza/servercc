project(servercc)

# Setup minimum required version of CMake and C++ standard.
cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)

# Create a library for the project.
add_library(${PROJECT_NAME} INTERFACE)

# Set the include directory.
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add subdirectories.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/client)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/connector)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/distributed)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/server)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/types)

# Link libraries to the project.
target_link_libraries(
    ${PROJECT_NAME}
    INTERFACE
        client
        connector
        distributed
        server
        types
)

# If a user has not specified a directory for libcc, then clone it from GitHub into lib/libcc.
if (NOT DEFINED LIBCC_DIR)
    set(LIBCC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/libcc)
    if (NOT EXISTS ${LIBCC_DIR})
        message(STATUS "Cloning libcc into ${LIBCC_DIR}")
        execute_process(COMMAND git clone https://github.com/OtavioPiza/libcc.git ${LIBCC_DIR})

    endif()

endif()

# Add libcc as a subdirectory.
add_subdirectory(${LIBCC_DIR})

# Run enable testing if this is the top level project.
if (${PROJECT_IS_TOP_LEVEL})
    enable_testing()
endif()
