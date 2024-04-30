protobuf_generate_cpp(PROTO_SRC PROTO_HEADER counter.proto)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(counter_client client.cpp ${PROTO_SRC} ${PROTO_HEADER})
add_executable(counter_server server.cpp ${PROTO_SRC} ${PROTO_HEADER})

target_link_libraries(counter_server braft-static)
target_link_libraries(counter_client braft-static)

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