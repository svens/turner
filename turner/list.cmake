list(APPEND turner
  turner/error.hpp
  turner/error.cpp
  turner/message.hpp
  turner/message.cpp

  # MS-TURN
  turner/msturn.hpp
  turner/msturn/message.hpp

  # STUN
  turner/stun.hpp
  turner/stun/message.hpp

  # TURN
  turner/turn.hpp
  turner/turn/message.hpp
)

list(APPEND turner_unittests
  turner/common.test.hpp
  turner/error.test.cpp
  turner/message.test.cpp

  # MS-STUN
  turner/msturn/message.test.cpp

  # STUN
  turner/stun/message.test.cpp

  # TURN
  turner/turn/message.test.cpp
)

include(turner/client/list.cmake)
include(turner/server/list.cmake)
