set(GBENCH_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tps/gbench/include")

if(NOT EXISTS ${GBENCH_INCLUDE_DIR}/benchmark/benchmark.h)
  find_package(Git)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} submodule update --init tps/gbench
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif()

include_directories(SYSTEM ${GBENCH_INCLUDE_DIR})

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark library testing")
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "Disable benchmark library installing")
add_subdirectory(tps/gbench)

if(CMAKE_COMPILER_IS_GNUCXX OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
  target_compile_options(benchmark
    PRIVATE -Wno-effc++
  )
endif()
