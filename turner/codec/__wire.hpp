#pragma once

/**
 * \file turner/codec/__wire.hpp
 * Wire layout pieces common to all protocols
 * \internal
 */

#include <pal/packed_struct.hpp>
#include <cstddef>
#include <cstdint>

namespace turner::__wire
{

struct type_and_length
{
	pal::big_endian<uint16_t> type;
	pal::big_endian<uint16_t> length;
};

static_assert(pal::packed_struct<type_and_length>);

inline constexpr uint16_t type_reserved_bits = 0b1100'0000'0000'0000;

constexpr size_t align_up (size_t size, size_t alignment) noexcept
{
	return (size + alignment - 1) & ~(alignment - 1);
}

} // namespace turner::__wire
