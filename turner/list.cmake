list(APPEND turner_sources
	turner/__bits/lib
	turner/__bits/view
	turner/attribute_type
	turner/attribute_value_type
	turner/basic_message
	turner/basic_message_reader
	turner/error
	turner/error.cpp
	turner/fwd
	turner/msturn
	turner/msturn.cpp
	turner/protocol_error
	turner/protocol_error.cpp
	turner/stun
	turner/stun.cpp
	turner/turn
)

list(APPEND turner_test_sources
	turner/test
	turner/test.cpp
	turner/attribute_type.test.cpp
	turner/attribute_value_type.test.cpp
	turner/basic_message.test.cpp
	turner/basic_message_reader.test.cpp
	turner/error.test.cpp
	turner/msturn.test.cpp
	turner/protocol_error.test.cpp
	turner/stun.test.cpp
	turner/turn.test.cpp
)
