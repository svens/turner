list(APPEND turner_sources
  turner/msturn/msturn.hpp
  turner/msturn/attributes.hpp
  turner/msturn/errors.hpp
  turner/msturn/messages.hpp
  turner/msturn/protocol.hpp
)

list(APPEND turner_unittests_sources
  turner/msturn/attributes.test.cpp
  turner/msturn/messages.test.cpp
)
