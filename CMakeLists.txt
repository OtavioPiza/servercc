cmake_minimum_required(VERSION 3.2)

project(rest++ VERSION 1.0.0)

if (!UNIX)
    message(FATAL_ERROR "This project is only supported on UNIX")
endif()

# configure c++ standard and flags

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

#  executables

add_executable("${PROJECT_NAME}" main.cc)

# libs

add_subdirectory(llhttp)
target_link_libraries(${PROJECT_NAME} llhttp)

add_subdirectory(server)
target_link_libraries("${PROJECT_NAME}" server)
