list(APPEND turner
  turner/error.hpp
  turner/error.cpp
  turner/message.hpp
  turner/message.cpp

  # STUN
  turner/stun.hpp
  turner/stun/message.hpp
  turner/stun/message.cpp
)

list(APPEND turner_unittests
  turner/common.test.hpp
  turner/error.test.cpp
  turner/message.test.cpp

  # STUN
  turner/stun/message.test.cpp
)

include(turner/client/list.cmake)
include(turner/server/list.cmake)
