list(APPEND turner
  turner/stun.hpp
  turner/stun.cpp
)

list(APPEND turner_unittests
  turner/common.test.hpp
  turner/stun.test.cpp
)

include(turner/client/list.cmake)
include(turner/server/list.cmake)
