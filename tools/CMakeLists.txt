set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/output/bin)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(braft_cli braft_cli.cpp)
target_link_libraries(braft_cli braft-static)
      