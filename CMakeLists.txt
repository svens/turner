project(turner)
cmake_minimum_required(VERSION 2.8.10)


# Settings {{{1

# optional features
option(TURNER_UNITTESTS "Build unittests" ON)
option(TURNER_DOCS "Generate documentation" OFF)

if(CMAKE_BUILD_TYPE MATCHES Coverage)
  # special case for coverage build
  set(CMAKE_BUILD_TYPE "Debug")
  set(TURNER_COVERAGE ON)
  set(TURNER_UNITTESTS ON)
endif()


# Version {{{1

# read and parse
file(STRINGS VERSION version LIMIT_COUNT 1)
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
elseif(MSGC)
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


# Targets {{{1

# turner library
include(turner/list.cmake)
add_library(turner ${turner})

# turner_client executable
include(client/list.cmake)
add_executable(turner_client ${turner_client})
target_link_libraries(turner_client turner)

# turner_server executable
include(server/list.cmake)
add_executable(turner_server ${turner_server})
target_link_libraries(turner_server turner)


# Testing {{{1
if(TURNER_UNITTESTS)
  message(STATUS "Building unittests")

  # configure gtest
  find_package(Threads)
  find_package(GTest QUIET)
  if(NOT GTEST_FOUND)
    message(STATUS "Building own gtest")

    include(ExternalProject)
    ExternalProject_Add(gtest
      URL https://github.com/google/googletest/archive/release-1.7.0.zip
      URL_HASH SHA256=b58cb7547a28b2c718d1e38aee18a3659c9e3ff52440297e965f5edffe34b6d0
      PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest-1.7.0
      CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      INSTALL_COMMAND ""
      LOG_DOWNLOAD ON
      LOG_CONFIGURE ON
      LOG_BUILD ON
    )

    ExternalProject_Get_Property(gtest source_dir)
    set(GTEST_INCLUDE_DIRS ${source_dir}/include)

    ExternalProject_Get_Property(gtest binary_dir)
    link_directories(${binary_dir})
    set(GTEST_BOTH_LIBRARIES
      ${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}
      ${CMAKE_STATIC_LIBRARY_PREFIX}gtest_main${CMAKE_STATIC_LIBRARY_SUFFIX}
    )
  endif()

  # build turner tests
  enable_testing()
  add_executable(turner_unittests ${turner_unittests})
  include_directories(${GTEST_INCLUDE_DIRS})
  if(NOT GTEST_FOUND)
    add_dependencies(turner_unittests gtest)
  endif()
  target_link_libraries(turner_unittests
    turner
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
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


# }}}1
