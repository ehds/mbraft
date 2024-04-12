find_package(Gperftools)
include(thirdparty/gtest/SetupGtest)

include_directories(${GPERFTOOLS_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/test)

set(CMAKE_CPP_FLAGS "-DGFLAGS_NS=${GFLAGS_NS}")
set(CMAKE_CPP_FLAGS "${CMAKE_CPP_FLAGS} -D__const__=__unused__ -D_GNU_SOURCE -DUSE_SYMBOLIZE -DNO_TCMALLOC -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -DUNIT_TEST -g -Dprivate=public -Dprotected=public -D__STRICT_ANSI__ -include sstream_workaround.h")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CPP_FLAGS} -O2 -pipe -Wall -W -fPIC -fstrict-aliasing -Wno-invalid-offsetof -Wno-unused-parameter -fno-omit-frame-pointer -Wno-unused-result")

if (WITH_COVERAGE)
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        link_libraries(gcov)
    else()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    endif()
    
endif()

# bthread_* functions are used in logging.cc, and they need to be marked as
# weak symbols explicitly in Darwin system.
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(DYNAMIC_LIB ${DYNAMIC_LIB}
        pthread
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework CoreData"
        "-framework CoreText"
        "-framework Security"
        "-framework Foundation"
        "-Wl,-U,_MallocExtension_ReleaseFreeMemory"
        "-Wl,-U,_ProfilerStart"
        "-Wl,-U,_ProfilerStop"
        "-Wl,-U,__Z13GetStackTracePPvii")
endif()

file(GLOB TEST_BRAFT_SRCS "test_*.cpp")
foreach(BRAFT_UT ${TEST_BRAFT_SRCS})
    get_filename_component(BRAFT_UT_WE ${BRAFT_UT} NAME_WE)
    add_executable(${BRAFT_UT_WE} ${BRAFT_UT})
    target_link_libraries(${BRAFT_UT_WE}
                          ${GTEST_MAIN_LIB}
                          ${GTEST_LIB}
                          ${GPERFTOOLS_LIBRARY}
                          ${DYNAMIC_LIB}
                          $<TARGET_OBJECTS:OBJ_LIB>
                          )
endforeach()
