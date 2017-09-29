find_path(SAL_INCLUDE_DIR sal/config.hpp)
find_library(SAL_LIB sal)

if(NOT SAL_INCLUDE_DIR OR NOT SAL_LIB)
  message(STATUS "Building own sal")

  include(ExternalProject)
  ExternalProject_Add(sal
    URL https://github.com/svens/sal/archive/v0.15.0.zip
    URL_HASH SHA256=c9a1d4b87a134391cdc9a45113c31e57415d61d89fba029eaf8a7d937eaa6e16
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/sal-0.15.0
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DSAL_UNITTESTS=no
      -DSAL_BENCH=no
      -DSAL_DOCS=no
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
  )

  ExternalProject_Get_Property(sal binary_dir)
  link_directories(${binary_dir})
  set(SAL_LIB ${CMAKE_STATIC_LIBRARY_PREFIX}sal${CMAKE_STATIC_LIBRARY_SUFFIX})

  ExternalProject_Get_Property(sal source_dir)
  set(SAL_INCLUDE_DIR ${source_dir} ${binary_dir})
endif()

include_directories(${SAL_INCLUDE_DIR})
