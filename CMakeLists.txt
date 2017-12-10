cmake_minimum_required(VERSION 3.2)

project(turner)
include(cmake/project_version.cmake)
message(STATUS "Turner: ${PROJECT_VERSION}")


# Settings {{{1

# optional features
option(turner_unittests "Build unittests" ON)
option(turner_bench "Build benchmarking application" OFF)
option(turner_docs "Generate documentation" OFF)

if(CMAKE_BUILD_TYPE MATCHES Coverage)
  # special case for coverage build
  set(CMAKE_BUILD_TYPE "Debug")
  set(COVERAGE ON)
  set(turner_unittests ON)
  set(turner_bench OFF)
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
add_library(turner ${turner_sources})
add_dependencies(turner sal)


# Testing {{{1
if(turner_unittests)
  message(STATUS "Building unittests")
  include(cmake/gtest.cmake)

  enable_testing()

  add_executable(unittests ${turner_unittests_sources})
  target_link_libraries(unittests
    turner
    sal
    gtest_main
    ${TURNER_DEP_LIBS}
  )
  add_dependencies(unittests gtest)

  add_test(unittests unittests)
endif()


# Benchmarking {{{1
if(turner_bench)
  message(STATUS "Building benchmarking applications")
  include(cmake/gbench.cmake)

  include(bench/list.cmake)
  add_executable(bench ${turner_bench_sources})
  target_link_libraries(bench
    turner
    sal
    benchmark
    ${TURNER_DEP_LIBS}
  )
  add_dependencies(bench benchmark)
endif()


# Documentation {{{1
if(turner_docs)
  include(cmake/doxygen.cmake)
endif()


# }}}1
