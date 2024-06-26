if(WITH_TESTS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNIT_TEST")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUNIT_TEST")
elseif(NOT DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNDEBUG")
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

add_library(OBJ_LIB OBJECT ${SOURCES})

set_property(TARGET ${OBJ_LIB} PROPERTY POSITION_INDEPENDENT_CODE 1)

if(WITH_TESTS AND WITH_COVERAGE)
    target_compile_options(OBJ_LIB PRIVATE -g -O0 --coverage -fprofile-arcs -ftest-coverage)
endif()

add_library(braft-shared SHARED $<TARGET_OBJECTS:OBJ_LIB>)
add_library(braft-static STATIC $<TARGET_OBJECTS:OBJ_LIB>)
target_link_libraries(braft-shared ${DYNAMIC_LIB})
target_link_libraries(braft-static ${DYNAMIC_LIB})

SET_TARGET_PROPERTIES(braft-static PROPERTIES OUTPUT_NAME braft CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(braft-shared PROPERTIES OUTPUT_NAME braft CLEAN_DIRECT_OUTPUT 1)

install(TARGETS braft-static
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib${LIBSUFFIX}
        ARCHIVE DESTINATION lib${LIBSUFFIX}
        )

install(TARGETS braft-shared
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib${LIBSUFFIX}
        ARCHIVE DESTINATION lib${LIBSUFFIX}
        )
