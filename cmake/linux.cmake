#
# Linux
#

# dependencies
find_package(OpenSSL REQUIRED)
list(FIND CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES OPENSSL_INCLUDE_DIR i)
if(${i} EQUAL -1)
  # not already in include path, add
  include_directories(${OPENSSL_INCLUDE_DIR})
endif()
list(APPEND TURNER_DEP_LIBS ${OPENSSL_CRYPTO_LIBRARY})
