find_path(SAL_INCLUDE_DIR sal/config.hpp)
find_library(SAL_LIB sal)

if(NOT SAL_INCLUDE_DIR OR NOT SAL_LIB)
  message(STATUS "Building own sal")

  include(ExternalProject)
  ExternalProject_Add(sal
    URL https://github.com/svens/sal/archive/v0.25.0.zip
    URL_HASH SHA256=d3db7311392cef7ff0996e43e2b10fcd7a150aa4198a70da3d453fb27ab4d8b5
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/sal-0.25.0
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -Dsal_unittests=no
      -Dsal_bench=no
      -Dsal_docs=no
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
  )

  # sal library
  ExternalProject_Get_Property(sal binary_dir)
  link_directories(${binary_dir})
  set(SAL_LIB ${CMAKE_STATIC_LIBRARY_PREFIX}sal${CMAKE_STATIC_LIBRARY_SUFFIX})

  # sal headers
  ExternalProject_Get_Property(sal source_dir)
  set(SAL_INCLUDE_DIR ${source_dir} ${binary_dir})
  include_directories(${SAL_INCLUDE_DIR})
endif()
