list(APPEND samples_common_sources
	samples/command_line.hpp
)

cxx_executable(stun_binding
	SOURCES ${samples_common_sources}
		samples/stun_binding.cpp
	LIBRARIES turner::protocol
)
