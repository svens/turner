list(APPEND turner_sources
  turner/turn/turn.hpp
  turner/turn/attributes.hpp
  turner/turn/errors.hpp
  turner/turn/messages.hpp
  turner/turn/protocol.hpp
)

list(APPEND turner_unittests_sources
  turner/turn/attributes.test.cpp
  turner/turn/messages.test.cpp
  turner/turn/protocol.test.cpp
)
