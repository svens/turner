list(APPEND turner
  turner/stun.hpp
  turner/stun/attributes.hpp
  turner/stun/attributes.cpp
  turner/stun/errors.hpp
  turner/stun/messages.hpp
  turner/stun/protocol.hpp
)

list(APPEND turner_unittests
  turner/stun/attributes.test.cpp
  turner/stun/messages.test.cpp
)
