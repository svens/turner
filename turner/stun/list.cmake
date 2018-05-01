list(APPEND turner_sources
  turner/stun/stun.hpp
  turner/stun/attributes.hpp
  turner/stun/client.hpp
  turner/stun/errors.hpp
  turner/stun/messages.hpp
  turner/stun/protocol.hpp
)

list(APPEND turner_unittests_sources
  turner/stun/attributes.test.cpp
  turner/stun/messages.test.cpp
  turner/stun/protocol.test.cpp
)
