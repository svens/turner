find_path(GBENCH_INCLUDE_DIR benchmark/benchmark.h)
find_library(GBENCH_LIBS benchmark)

if(GBENCH_INCLUDE_DIR AND GBENCH_LIBS)
  set(GBENCH_FOUND "yes")
  if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND GBENCH_LIBS c++)
  endif()
else()
  message(STATUS "Building own google-benchmark")

  include(ExternalProject)
  ExternalProject_Add(benchmark
    URL https://github.com/google/benchmark/archive/v1.3.0.zip
    URL_HASH SHA256=51c2d2d35491aea83aa6121afc4a1fd9262fbd5ad679eb5e03c9fa481e42571e
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gbench-1.3.0
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}
      -DBENCHMARK_ENABLE_TESTING=no
      -DBENCHMARK_ENABLE_INSTALL=no
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
  )

  ExternalProject_Get_Property(benchmark source_dir)
  include_directories(${source_dir}/include)

  ExternalProject_Get_Property(benchmark binary_dir)
  link_directories(${binary_dir}/src)
  set(GBENCH_LIBS
    ${CMAKE_STATIC_LIBRARY_PREFIX}benchmark${CMAKE_STATIC_LIBRARY_SUFFIX}
  )
  if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    list(APPEND GBENCH_LIBS "shlwapi")
  endif()
endif()
