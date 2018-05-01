list(APPEND turner_sources
  turner/__bits/attribute_processor.hpp
  turner/__bits/attribute_processor.cpp
  turner/__bits/message.hpp
  turner/__bits/message.cpp
  turner/agent_base.hpp
  turner/attribute.hpp
  turner/attribute_processor.hpp
  turner/basic_agent.hpp
  turner/error.hpp
  turner/error.cpp
  turner/fwd.hpp
  turner/message.hpp
  turner/message_type.hpp
  turner/protocol.hpp
)

list(APPEND turner_unittests_sources
  turner/common.test.hpp
  turner/attribute.test.cpp
  turner/attribute_processor.test.cpp
  turner/basic_agent.test.cpp
  turner/error.test.cpp
  turner/message.test.cpp
  turner/message_type.test.cpp
  turner/protocol.test.cpp
)

# protocols
include(turner/msturn/list.cmake)
include(turner/stun/list.cmake)
include(turner/turn/list.cmake)

# transport
include(turner/transport/list.cmake)
