#pragma once

/**
 * \file turner/codec/__wire.hpp
 * Wire layout pieces common to all protocols
 * \internal
 */

#include <pal/packed_struct.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>

namespace turner::__wire
{

struct type_and_length
{
	pal::big_endian<uint16_t> type;
	pal::big_endian<uint16_t> length;

	static const type_and_length &read (std::span<const std::byte> bytes) noexcept
	{
		return pal::packed_as<type_and_length>(bytes.first<sizeof(type_and_length)>());
	}

	static void write (std::span<std::byte> bytes, uint16_t type, size_t length) noexcept
	{
		pal::packed_as<type_and_length>(bytes.first<sizeof(type_and_length)>()) =
		{
			.type = type,
			.length = static_cast<uint16_t>(length),
		};
	}
};

static_assert(pal::packed_struct<type_and_length>);

inline constexpr size_t attribute_header_size = sizeof(type_and_length);

inline constexpr uint16_t type_reserved_bits = 0b1100'0000'0000'0000;

constexpr bool comprehension_required (uint16_t attribute_type) noexcept
{
	return attribute_type < 0x8000;
}

constexpr size_t align_up (size_t size, size_t alignment) noexcept
{
	return (size + alignment - 1) & ~(alignment - 1);
}

inline size_t offset_of (std::span<const std::byte> part, std::span<const std::byte> whole) noexcept
{
	return static_cast<size_t>(part.data() - whole.data());
}

template <typename Protocol>
constexpr size_t attributes_offset = (std::max)(
	Protocol::header_size,
	Protocol::cookie_offset + Protocol::cookie.size()
);

} // namespace turner::__wire
