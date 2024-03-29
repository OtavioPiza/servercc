add_library(connector ${CMAKE_CURRENT_SOURCE_DIR}/src/connector.cc)
target_include_directories(
    connector
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(
    connector
    PRIVATE
        absl::flat_hash_map
        absl::log
        absl::status
        absl::strings
        clients
        types
    PUBLIC
        libcc   # TODO: figure out how to make this private
)


add_library(connectors INTERFACE)
target_include_directories(
    connector
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
    connectors
    INTERFACE
        connector
)


add_library(internal_channel ${CMAKE_CURRENT_SOURCE_DIR}/src/internal_channel.cc)
target_include_directories(
    internal_channel
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(
    internal_channel
    PRIVATE
        absl::log
        absl::status
        types
    PUBLIC
        libcc   # TODO: figure out how to make this private
)


add_library(internal_channel_manager ${CMAKE_CURRENT_SOURCE_DIR}/src/internal_channel_manager.cc)
target_include_directories(
    internal_channel_manager
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(
    internal_channel_manager
    PRIVATE
        absl::log
        absl::status
        internal_channel
        types
    PUBLIC
        libcc   # TODO: figure out how to make this private
)
