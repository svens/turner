list(APPEND turner
  turner/attribute.hpp
  turner/attribute_processor.hpp
  turner/error.hpp
  turner/error.cpp
  turner/fwd.hpp
  turner/message.hpp
  turner/message_type.hpp
  turner/protocol.hpp

  # specific protocol headers
  turner/stun.hpp
  turner/stun.cpp

  # internals
  turner/__bits/attribute_processor.hpp
  turner/__bits/attribute_processor.cpp
  turner/__bits/helpers.hpp
)

list(APPEND turner_unittests
  turner/common.test.hpp
  turner/attribute.test.cpp
  turner/attribute_processor.test.cpp
  turner/error.test.cpp
  turner/message.test.cpp
  turner/message_type.test.cpp
  turner/protocol.test.cpp

  # STUN tests
  turner/stun.test.cpp
)
