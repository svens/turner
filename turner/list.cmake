list(APPEND turner_sources
	turner/__bits/lib
	turner/error
	turner/error.cpp
	turner/message_type
	turner/protocol_error
	turner/protocol_error.cpp
)

list(APPEND turner_test_sources
	turner/test
	turner/test.cpp
	turner/error.test.cpp
	turner/message_type.test.cpp
	turner/protocol_error.test.cpp
)
