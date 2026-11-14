cmake_minimum_required(VERSION 3.28)

project(turner LANGUAGES CXX)
include(cmake/project_version.cmake)
message(STATUS "${PROJECT_NAME} ${PROJECT_VERSION}")

option(TURNER_SERVER "Build server" ${PROJECT_IS_TOP_LEVEL})
option(TURNER_CLIENT "Build clients" ${PROJECT_IS_TOP_LEVEL})
option(TURNER_APPS "Build apps" ${PROJECT_IS_TOP_LEVEL})

if(TURNER_APPS)
	set(TURNER_SERVER ON)
endif()

include(cmake/cxx.cmake)
include(cmake/pal.cmake)

include(turner/list.cmake)
configure_file(
	turner/version.cpp.in
	version.cpp
	@ONLY
)
list(APPEND turner_codec_sources
	${CMAKE_CURRENT_BINARY_DIR}/version.cpp
)
add_library(turner_codec ${turner_codec_sources})
add_library(turner::codec ALIAS turner_codec)

target_include_directories(turner_codec PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(turner_codec PUBLIC pal::pal PRIVATE turner_cxx_warnings)

list(APPEND turner_test_sources ${turner_codec_test_sources})
list(APPEND turner_test_libraries turner::codec)

if(TURNER_SERVER)
	add_library(turner_server INTERFACE ${turner_server_sources})
	add_library(turner::server ALIAS turner_server)
	target_link_libraries(turner_server INTERFACE turner::codec)

	list(APPEND turner_test_sources ${turner_server_test_sources})
	list(APPEND turner_test_libraries turner::server)
endif()

if(TURNER_CLIENT)
	add_library(turner_simple_client INTERFACE ${turner_simple_client_sources})
	add_library(turner::simple_client ALIAS turner_simple_client)
	target_link_libraries(turner_simple_client INTERFACE turner::codec)

	list(APPEND turner_test_sources ${turner_simple_client_test_sources})
	list(APPEND turner_test_libraries turner::simple_client)
endif()

if(TURNER_APPS)
	include(app/list.cmake)

	add_executable(turner-server ${turner_app_server_sources})
	target_link_libraries(turner-server PRIVATE turner_cxx_warnings turner::server)
endif()

if(PROJECT_IS_TOP_LEVEL)
	include(cmake/tidy.cmake)
	include(cmake/test.cmake)

	add_executable(turner_test ${turner_test_sources})
	target_link_libraries(turner_test PRIVATE turner_cxx_test ${turner_test_libraries})
	set_target_properties(turner_test PROPERTIES LINK_LIBRARIES_STRATEGY REORDER_FREELY)
	add_test(NAME turner_test COMMAND turner_test --skip-benchmarks)
endif()
