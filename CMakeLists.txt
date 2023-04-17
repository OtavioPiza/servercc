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

# Setup libcc if it is not already setup.
if (NOT TARGET libcc)

    # If a user has not specified a directory for libcc, then clone it from GitHub into lib/libcc.
    if (NOT DEFINED LIBCC_DIR)
        # Set the directory to clone libcc into.
        set(LIBCC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/libcc)

        # If the directory does not exist, then clone it.
        if (NOT EXISTS ${LIBCC_DIR})
            message(STATUS "Cloning libcc into ${LIBCC_DIR}")
            execute_process(COMMAND git clone https://github.com/OtavioPiza/libcc.git ${LIBCC_DIR})

        # If the directory does exist, then update it.
        else()
            message(STATUS "Updating libcc in ${LIBCC_DIR}")
            execute_process(COMMAND git pull origin main WORKING_DIRECTORY ${LIBCC_DIR})

        endif()

    endif()

    # Add libcc as a subdirectory with a build directory inside of LIBCC_DIR.
    add_subdirectory(${LIBCC_DIR} ${LIBCC_DIR}/build)

endif()

# Run enable testing if this is the top level project.
if (${PROJECT_IS_TOP_LEVEL})
    enable_testing()
endif()
