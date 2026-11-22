list(APPEND turner_codec_sources
	turner/uri.hpp
	turner/uri.cpp
	turner/version.hpp
)

list(APPEND turner_codec_test_sources
	turner/uri.test.cpp
	turner/version.test.cpp
)

list(APPEND turner_codec_fuzz_sources
	turner/uri.fuzz.cpp
)

include(turner/codec/list.cmake)
include(turner/server/list.cmake)
include(turner/simple_client/list.cmake)

list(APPEND turner_fuzz_sources
	${turner_codec_fuzz_sources}
)
