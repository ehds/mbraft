protobuf_generate_cpp(PROTO_SRC PROTO_HEADER block.proto)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(block_client client.cpp ${PROTO_SRC} ${PROTO_HEADER})
add_executable(block_server server.cpp ${PROTO_SRC} ${PROTO_HEADER})

target_link_libraries(block_server braft-static)
target_link_libraries(block_client braft-static)

# Copy start/stop scripts
# Copy start/stop scripts
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/
    FILES_MATCHING
    PATTERN "run_client.sh"
    PATTERN "run_server.sh"
    PATTERN "stop.sh"
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../shflags
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)