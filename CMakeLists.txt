cmake_minimum_required(VERSION 3.12)

project(turner LANGUAGES CXX)
include(cmake/project_version.cmake)
message(STATUS "${CMAKE_PROJECT_NAME} ${CMAKE_PROJECT_VERSION}")

option(turner_test "Build tests" ON)
option(turner_doc "Generate documentation" OFF)
option(turner_samples "Build samples" OFF)

if(NOT "${CMAKE_BUILD_TYPE};${CMAKE_CONFIGURATION_TYPES}" MATCHES ".*Coverage.*")
	set(turner_test ON)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# subtarget option() honors normal variables
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(pal_test OFF)
set(pal_doc OFF)
set(pal_samples OFF)
add_subdirectory(pal)

include(cmake/cxx.cmake)

# library {{{1
include(turner/list.cmake)
cxx_library(turner
	SOURCES ${turner_sources}
	LIBRARIES pal::pal
)

# tests (+coverage) {{{1
if(turner_test)
	cxx_test(turner_test
		SOURCES ${turner_test_sources}
		LIBRARIES turner::turner
		COVERAGE_BASE_DIR ${CMAKE_SOURCE_DIR}/turner
	)
endif()

# documentation {{{1
if(turner_doc)
	cxx_doc(turner
		MAIN_PAGE ${CMAKE_SOURCE_DIR}/README.md
		DIRECTORY ${CMAKE_SOURCE_DIR}/turner
	)
endif()

# samples {{{1
if(turner_samples)
	include(samples/list.cmake)
endif()
