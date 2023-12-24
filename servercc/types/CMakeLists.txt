add_library(message_lib ${CMAKE_CURRENT_SOURCE_DIR}/src/message.cc)
target_include_directories(
    message_lib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(
    message_lib
    PRIVATE
        absl::status
)


add_library(types INTERFACE)
target_include_directories(
    types
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    types
    INTERFACE
        absl::status
        absl::strings
        message_lib
)
