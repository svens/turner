list(APPEND turner_codec_sources
	turner/codec/method.hpp
	turner/codec/msturn.hpp
	turner/codec/protocol.hpp
	turner/codec/stun.hpp
	turner/codec/turn.hpp
)

list(APPEND turner_codec_test_sources
	turner/codec/method.test.cpp
	turner/codec/msturn.test.cpp
	turner/codec/stun.test.cpp
	turner/codec/turn.test.cpp
)
