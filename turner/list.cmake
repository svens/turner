list(APPEND turner
  turner/error.hpp
  turner/error.cpp
  turner/fwd.hpp
  turner/message.hpp
  turner/message_type.hpp
  turner/protocol.hpp

  # specific protocol headers
  turner/stun.hpp
)

list(APPEND turner_unittests
  turner/common.test.hpp
  turner/error.test.cpp
  turner/message.test.cpp
  turner/message_type.test.cpp
  turner/protocol.test.cpp
)
