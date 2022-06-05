#pragma once

#include <cstddef>
#include <cstdint>
#include <pal/__bits/lib>

// project namespace
#define __turner_begin namespace turner { inline namespace v0 {
#define __turner_end   }} // namespace turner::v0

__turner_begin

//
// build configuration
//

constexpr bool is_release_build =
#if NDEBUG
	true
#else
	false
#endif
;

__turner_end
