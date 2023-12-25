# Setup minimum required version of CMake and C++ standard.
cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 20)

project(servercc)


# Configure the abseil library.
if (NOT TARGET absl AND NOT ABSL_ADDED)
    SET(ABSL_ADDED TRUE)

    # If a user has not specified a directory for abseil, then clone it from GitHub into lib/abseil-cpp.
    if (NOT DEFINED ABSL_DIR)
        # Set the directory to clone abseil into.
        set(ABSL_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib/abseil-cpp)

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


# Setup libcc if it is not already setup.
if (NOT TARGET libcc)

    # If a user has not specified a directory for libcc, then clone it from GitHub into lib/libcc.
    if (NOT DEFINED LIBCC_DIR)
        # Define the directory for libcc.
        set(LIBCC_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib/libcc)

        # If the directory does not exist, then clone libcc into it.
        if (NOT EXISTS ${LIBCC_DIR})
            message(STATUS "Cloning libcc into ${LIBCC_DIR}")
            execute_process(COMMAND git clone https://github.com/OtavioPiza/libcc.git ${LIBCC_DIR})

        # If the directory exists, then update it.
        else()
            message(STATUS "Updating libcc in ${LIBCC_DIR}")
            execute_process(COMMAND git pull origin main WORKING_DIRECTORY ${LIBCC_DIR})

        endif()

    endif()

    # Add libcc as a subdirectory and link it to the executable.
    add_subdirectory(${LIBCC_DIR} ${CMAKE_CURRENT_BINARY_DIR}/libcc)

endif()


# Add subdirectories.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/servercc/clients clients)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/servercc/connectors connectors)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/servercc/distributed)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/servercc/servers servers)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/servercc/types types)


# Create a library for the project.
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
    ${PROJECT_NAME}
    INTERFACE
        clients
        connectors
        distributed
        servers
        types
)

# Run enable testing if this is the top level project and build the executable main.cc
if (${PROJECT_IS_TOP_LEVEL})
    message(STATUS "Enabling testing for ${PROJECT_NAME}")
    enable_testing()

endif()

# Build demos if this is the top level project and BUILD_DEMOS is set to ON.
if (${PROJECT_IS_TOP_LEVEL} AND BUILD_DEMOS)
    message(STATUS "Building demos for ${PROJECT_NAME}")

    add_executable(distributed_demo demos/distributed_demo.cc)
    target_link_libraries(
        distributed_demo
            ${PROJECT_NAME}
            absl::log
            absl::log_globals
            absl::log_initialize
            absl::log_severity
    )

    add_executable(multicast_client_demo demos/multicast_client_demo.cc)
    target_link_libraries(
        multicast_client_demo
            ${PROJECT_NAME}
            absl::log_globals
            absl::log_initialize
            absl::log_severity
            absl::strings
    )

    add_executable(tcp_client_demo demos/tcp_client_demo.cc)
    target_link_libraries(
        tcp_client_demo
            ${PROJECT_NAME}
            absl::log_globals
            absl::log_initialize
            absl::log_severity
    )

    add_executable(tcp_server_demo demos/tcp_server_demo.cc)
    target_link_libraries(
        tcp_server_demo
            ${PROJECT_NAME}
            absl::log_globals
            absl::log_initialize
            absl::log_severity
    )

    add_executable(udp_server_demo demos/udp_server_demo.cc)
    target_link_libraries(
        udp_server_demo
            ${PROJECT_NAME}
            absl::log_globals
            absl::log_initialize
            absl::log_severity
    )

endif()
