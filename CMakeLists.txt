cmake_minimum_required(VERSION 3.2)

project(turner)
include(cmake/version.cmake)
message(STATUS "Version: ${PROJECT_VERSION}")


# Settings {{{1

# optional features
option(UNITTESTS "Build unittests" ON)
option(BENCH "Build benchmarking application" OFF)
option(DOCS "Generate documentation" OFF)

if(CMAKE_BUILD_TYPE MATCHES Coverage)
  # special case for coverage build
  set(CMAKE_BUILD_TYPE "Debug")
  set(COVERAGE ON)
  set(UNITTESTS ON)
  set(BENCH OFF)
endif()


# Build settings {{{1

# generate config header
message(STATUS "Generate turner/config.hpp")
configure_file(
  ${PROJECT_SOURCE_DIR}/turner/config.hpp.in
  ${CMAKE_BINARY_DIR}/turner/config.hpp
)
set_source_files_properties(${CMAKE_BINARY_DIR}/turner/config.hpp
  PROPERTIES GENERATED TRUE
)

# compiler
include_directories(. ${CMAKE_BINARY_DIR})
if(CMAKE_COMPILER_IS_GNUCXX)
  include(cmake/g++.cmake)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  include(cmake/clang++.cmake)
elseif(MSVC)
  include(cmake/msvc.cmake)
else()
  message(WARNING "Unknown compiler")
endif()

# system
if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  include(cmake/macos.cmake)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  include(cmake/linux.cmake)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  include(cmake/windows.cmake)
else()
  message(WARNING "Unknown system")
endif()

# dependent libraries
include(cmake/sal.cmake)


# Targets {{{1

# turner library
include(turner/list.cmake)
add_library(turner ${turner})
add_dependencies(turner sal)


# Testing {{{1
if(UNITTESTS)
  message(STATUS "Building unittests")
  include(cmake/gtest.cmake)

  # build turner tests
  enable_testing()
  add_executable(turner_unittests ${turner_unittests})
  include_directories(${GTEST_INCLUDE_DIRS})
  if(NOT GTEST_FOUND)
    add_dependencies(turner_unittests gtest)
  endif()

  # relax compiler warnings for gtest, intentionally creating unhappy paths
  if(CMAKE_COMPILER_IS_GNUCXX)
    set_source_files_properties(${turner_unittests}
      PROPERTIES COMPILE_FLAGS "-Wno-effc++"
    )
  elseif(MSVC)
    set_source_files_properties(${turner_unittests}
      PROPERTIES COMPILE_FLAGS "/analyze-"
    )
  endif()

  target_link_libraries(turner_unittests
    turner
    ${SAL_LIB}
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${TURNER_DEPS_LIBS}
  )
  add_test(unittests turner_unittests)
endif()


# Benchmarking {{{1
if(BENCH)
  message(STATUS "Building benchmarking applications")
  include(cmake/benchmark.cmake)

  include(benchmark/list.cmake)
  add_executable(turner_benchmark ${turner_benchmark})
  if(NOT GBENCH_FOUND)
    add_dependencies(turner_benchmark benchmark)
  endif()

  if(CMAKE_COMPILER_IS_GNUCXX)
    set_source_files_properties(${turner_benchmark}
      PROPERTIES COMPILE_FLAGS "-Wno-effc++"
    )
  elseif(MSVC)
    set_source_files_properties(${turner_benchmark}
      PROPERTIES COMPILE_FLAGS "/wd4141 /analyze-"
    )
  endif()

  target_link_libraries(turner_benchmark
    turner
    ${SAL_LIB}
    ${GBENCH_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    ${TURNER_DEPS_LIBS}
  )
endif()


# Documentation {{{1
if(DOCS)
  include(cmake/doxygen.cmake)
endif()


# }}}1
