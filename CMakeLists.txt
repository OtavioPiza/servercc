# Setup minimum required version of CMake and C++ standard.
cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 17)

project(servercc)

# Create a library for the project.
add_library(${PROJECT_NAME} INTERFACE)

# Set the include directory.
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# # Add subdirectories.
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/connector)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/distributed)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/servercc/clients clients)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/servercc/servers servers)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/servercc/types types)

# Link libraries to the project.
target_link_libraries(
    ${PROJECT_NAME}
    INTERFACE
        # connector
        # distributed
        clients
        servers
        types
)

# Configure the abseil library.
if (NOT TARGET absl AND NOT ABLS_ADDED)
    set(ABSL_PROPAGATE_CXX_STD ON)

    # If a user has not specified a directory for abseil, then clone it from GitHub into lib/abseil-cpp.
    if (NOT DEFINED ABSL_DIR)
        # Set the directory to clone abseil into.
        set(ABSL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/abseil-cpp)

        # If the directory does not exist, then clone it otherwise pull from origin master.
        if (NOT EXISTS ${ABSL_DIR})
            message(STATUS "Cloning abseil into ${ABSL_DIR}")
            execute_process(COMMAND git clone https://github.com/abseil/abseil-cpp.git ${ABSL_DIR})

        else()
            message(STATUS "Updating abseil in ${ABSL_DIR}")
            execute_process(COMMAND git pull origin master WORKING_DIRECTORY ${ABSL_DIR})

        endif()

    endif()

    # Add abseil as a subdirectory with a build directory of build with the same c++ standard
    # as this project.
    add_subdirectory(${ABSL_DIR} ${CMAKE_CURRENT_BINARY_DIR}/abseil-cpp)
endif()

# Run enable testing if this is the top level project and build the executable main.cc
if (${PROJECT_IS_TOP_LEVEL})
    message(STATUS "Enabling testing for ${PROJECT_NAME}")
    enable_testing()

endif()

# Build demos if this is the top level project and BUILD_DEMOS is set to ON.
if (${PROJECT_IS_TOP_LEVEL} AND BUILD_DEMOS)
    message(STATUS "Building demos for ${PROJECT_NAME}")

    add_executable(tcp_server_demo demos/tcp_server_demo.cc)
    target_link_libraries(tcp_server_demo ${PROJECT_NAME})

    add_executable(tcp_client_demo demos/tcp_client_demo.cc)
    target_link_libraries(tcp_client_demo ${PROJECT_NAME})
endif()
