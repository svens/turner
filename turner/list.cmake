list(APPEND turner_codec_sources
	turner/version.hpp
)

list(APPEND turner_codec_test_sources
	turner/version.test.cpp
)

include(turner/codec/list.cmake)
include(turner/server/list.cmake)
include(turner/simple_client/list.cmake)
