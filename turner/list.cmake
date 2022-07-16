list(APPEND turner_sources
	turner/__bits/lib
	turner/basic_message_reader
	turner/error
	turner/error.cpp
	turner/fwd
	turner/message_type
	turner/protocol_error
	turner/protocol_error.cpp
	turner/stun
)

list(APPEND turner_test_sources
	turner/test
	turner/test.cpp
	turner/error.test.cpp
	turner/message_type.test.cpp
	turner/protocol_error.test.cpp
	turner/stun.test.cpp
)
