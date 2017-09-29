list(APPEND turner
  turner/error.hpp
  turner/error.cpp
  turner/stun.hpp
  turner/stun.cpp
)

list(APPEND turner_unittests
  turner/common.test.hpp
  turner/error.test.cpp
  turner/stun.test.cpp
)

include(turner/client/list.cmake)
include(turner/server/list.cmake)
