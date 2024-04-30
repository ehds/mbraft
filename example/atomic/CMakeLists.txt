
protobuf_generate_cpp(PROTO_SRC PROTO_HEADER atomic.proto)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(atomic_client client.cpp ${PROTO_SRC} ${PROTO_HEADER})
add_executable(atomic_server server.cpp ${PROTO_SRC} ${PROTO_HEADER})
add_executable(atomic_test test.cpp ${PROTO_SRC} ${PROTO_HEADER})

target_link_libraries(atomic_client braft-static)
target_link_libraries(atomic_server braft-static)
target_link_libraries(atomic_test braft-static)

message("--- ${CMAKE_CURRENT_BINARY_DIR}")
message("--- ${CMAKE_CURRENT_SOURCE_DIR}")
message("--- ${CMAKE_CURRENT_LIST_DIR}")

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