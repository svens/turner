set(SAL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tps/sal")

if(NOT EXISTS ${SAL_INCLUDE_DIR}/sal/assert.hpp)
  find_package(Git)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} submodule update --init tps/sal
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif()

include_directories(${SAL_INCLUDE_DIR})

set(sal_unittests OFF CACHE BOOL "Disable sal library testing")
add_subdirectory(tps/sal)
