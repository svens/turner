project(turner)
cmake_minimum_required(VERSION 2.8.10)


# Settings {{{1

# optional features
option(TURNER_UNITTESTS "Build unittests" ON)
option(TURNER_DOCS "Generate documentation" OFF)


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
include(turner_client/list.cmake)
add_executable(turner_client ${turner_client})
target_link_libraries(turner_client turner)

# turner_server executable
include(turner_server/list.cmake)
add_executable(turner_server ${turner_server})
target_link_libraries(turner_server turner)

# }}}1
