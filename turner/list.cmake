list(APPEND turner_sources
	turner/__bits/lib.hpp
	turner/error.hpp
	turner/error.cpp
	turner/protocol_error.hpp
	turner/protocol_error.cpp
)

list(APPEND turner_test_sources
	turner/test.hpp
	turner/test.cpp
	turner/error.test.cpp
	turner/protocol_error.test.cpp
)
