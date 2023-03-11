list(APPEND turner_sources
	turner/__bits/lib
	turner/__bits/view
	turner/attribute_type
	turner/attribute_value_type
	turner/error
	turner/error.cpp
	turner/fwd
	turner/message_reader
	turner/message_type
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
	turner/error.test.cpp
	turner/message_reader.test.cpp
	turner/message_type.test.cpp
	turner/msturn.test.cpp
	turner/protocol_error.test.cpp
	turner/stun.test.cpp
	turner/turn.test.cpp
)
