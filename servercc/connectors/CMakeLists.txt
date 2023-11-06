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
        absl::status
        absl::strings
        clients
        types
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
