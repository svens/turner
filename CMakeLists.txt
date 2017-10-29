project(turner)
cmake_minimum_required(VERSION 2.8.10)


# Settings {{{1

# optional features
option(UNITTESTS "Build unittests" ON)
option(DOCS "Generate documentation" OFF)

if(CMAKE_BUILD_TYPE MATCHES Coverage)
  # special case for coverage build
  set(CMAKE_BUILD_TYPE "Debug")
  set(COVERAGE ON)
  set(UNITTESTS ON)
endif()


# Version {{{1

# read and parse
include(VERSION)
set(version_regex "([0-9]*)\\.([0-9]*)\\.([0-9]*)")
string(REGEX REPLACE ${version_regex} "\\1" version_major ${version})
math(EXPR v ${version_major}+0)
string(REGEX REPLACE ${version_regex} "\\2" version_minor ${version})
math(EXPR v ${version_minor}+0)
string(REGEX REPLACE ${version_regex} "\\3" version_patch ${version})
math(EXPR v ${version_patch}+0)

# generate config header
message(STATUS "Generate turner/config.hpp")
configure_file(
  ${PROJECT_SOURCE_DIR}/turner/config.hpp.in
  ${CMAKE_BINARY_DIR}/turner/config.hpp
)
set_source_files_properties(${CMAKE_BINARY_DIR}/turner/config.hpp
  PROPERTIES GENERATED TRUE
)


# Build settings {{{1

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
  target_link_libraries(turner_unittests
    turner
    ${SAL_LIB}
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${TURNER_DEPS_LIBS}
  )

  # relax compiler warnings for gtest, we don't control it
  if(CMAKE_COMPILER_IS_GNUCXX)
    set_source_files_properties(${turner_unittests}
      PROPERTIES COMPILE_FLAGS "-Wno-effc++"
    )
  elseif(MSVC)
    set_source_files_properties(${turner_unittests}
      PROPERTIES COMPILE_FLAGS "/analyze-"
    )
  endif()

  add_test(unittests turner_unittests)
endif()


# Documentation {{{1
if(DOCS)
  include(cmake/doxygen.cmake)
endif()


# }}}1
