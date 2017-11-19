list(APPEND turner
  turner/turn/turn.hpp
  turner/turn/attributes.hpp
  turner/turn/errors.hpp
  turner/turn/messages.hpp
  turner/turn/protocol.hpp
)

list(APPEND turner_unittests
  turner/turn/attributes.test.cpp
  turner/turn/messages.test.cpp
)
