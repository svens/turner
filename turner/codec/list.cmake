list(APPEND turner_codec_sources
	turner/codec/__wire.hpp
	turner/codec/attribute.hpp
	turner/codec/error.hpp
	turner/codec/error.cpp
	turner/codec/method.hpp
	turner/codec/msturn.hpp
	turner/codec/msturn.cpp
	turner/codec/protocol.hpp
	turner/codec/protocol_error.hpp
	turner/codec/protocol_error.cpp
	turner/codec/reader.hpp
	turner/codec/stun.hpp
	turner/codec/stun.cpp
	turner/codec/turn.hpp
	turner/codec/turn.cpp
	turner/codec/value.hpp
)

list(APPEND turner_codec_test_sources
	turner/codec/test.hpp
	turner/codec/attribute.test.cpp
	turner/codec/error.test.cpp
	turner/codec/method.test.cpp
	turner/codec/msturn.test.cpp
	turner/codec/protocol_error.test.cpp
	turner/codec/reader.test.cpp
	turner/codec/stun.test.cpp
	turner/codec/turn.test.cpp
	turner/codec/value.test.cpp
)
