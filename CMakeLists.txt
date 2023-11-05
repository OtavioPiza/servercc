# Setup minimum required version of CMake and C++ standard.
cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 20)

project(servercc)

# Create a library for the project.
add_library(${PROJECT_NAME} INTERFACE)

# Set the include directory.
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# # Add subdirectories.
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/connector)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/distributed)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/servercc/clients clients)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/servercc/types types)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/servercc/servers servers)

# Link libraries to the project.
target_link_libraries(
    ${PROJECT_NAME}
    INTERFACE
        # connector
        # distributed
        clients
        types
        servers
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

    # Add libcc as a subdirectory with a build directory of build.
    add_subdirectory(${LIBCC_DIR} ${CMAKE_CURRENT_BINARY_DIR}/libcc)

endif()

if (NOT TARGET absl AND NOT ABLS_ADDED)
    set(ABLS_ADDED TRUE)

    # If a user has not specified a directory for abseil, then clone it from GitHub into lib/abseil-cpp.
    if (NOT DEFINED ABSL_DIR)
        # Set the directory to clone abseil into.
        set(ABSL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/abseil-cpp)

        # If the directory does not exist, then clone it.
        if (NOT EXISTS ${ABSL_DIR})
            message(STATUS "Cloning abseil into ${ABSL_DIR}")
            execute_process(COMMAND git clone https://github.com/abseil/abseil-cpp.git ${ABSL_DIR})

        # If the directory does exist, then update it.
        else()
            message(STATUS "Updating abseil in ${ABSL_DIR}")
            execute_process(COMMAND git pull origin master WORKING_DIRECTORY ${ABSL_DIR})
        endif()

    endif()

    # Add abseil as a subdirectory with a build directory of build.
    add_subdirectory(${ABSL_DIR} ${CMAKE_CURRENT_BINARY_DIR}/lib/abseil-cpp)

endif()

# Run enable testing if this is the top level project and build the executable main.cc
if (${PROJECT_IS_TOP_LEVEL})
    enable_testing()

    add_executable(main main.cc)
    target_link_libraries(main types servers)
endif()
