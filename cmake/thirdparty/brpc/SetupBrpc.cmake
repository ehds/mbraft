# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Setup brpc
configure_file("${PROJECT_SOURCE_DIR}/cmake/thirdparty/brpc/CMakeLists.download_brpc.in" ${PROJECT_BINARY_DIR}/_deps/brpc/CMakeLists.txt)


execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/_deps/brpc)

if(result)
  message(FATAL_ERROR "CMake step for brpc failed: ${result}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/_deps/brpc)

if(result)
  message(FATAL_ERROR "Build step for brpc failed: ${result}")
endif()

add_subdirectory(${PROJECT_BINARY_DIR}/_deps/brpc/src
                 ${PROJECT_BINARY_DIR}/_deps/brpc/build
                 EXCLUDE_FROM_ALL)

set(BRPC_LIB brpc-static)
set(BRPC_INCLUDE_PATH  ${PROJECT_BINARY_DIR}/_deps/brpc/build/output/include)
