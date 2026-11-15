#include <turner/codec/stun.hpp>
#include <turner/codec/__wire.hpp>
#include <turner/codec/error.hpp>
#include <turner/codec/reader.hpp>
#include <pal/packed_struct.hpp>
#include <pal/result.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace turner
{

namespace
{

struct header
{
	pal::big_endian<uint16_t> type;
	pal::big_endian<uint16_t> length;
	std::array<std::byte, 4> cookie;
	stun::transaction_id transaction_id;
};

static_assert(pal::packed_struct<header>);
static_assert(sizeof(header) == stun::header_size);
static_assert(offsetof(header, cookie) == stun::cookie_offset);
static_assert(sizeof(header::cookie) == stun::cookie.size());
static_assert(offsetof(header, transaction_id) == stun::transaction_id_offset);

} // namespace

pal::result<parsed_message<stun>> stun::parse (std::span<std::byte> buffer, size_t received) noexcept
{
	if (received < header_size)
	{
		return make_unexpected(errc::incomplete);
	}

	const auto &h = pal::packed_as<header>(buffer.first<header_size>());
	if ((h.type & __wire::type_reserved_bits) != 0)
	{
		return make_unexpected(errc::unexpected_message_type);
	}
	if (h.cookie != cookie)
	{
		return make_unexpected(errc::invalid_magic_cookie);
	}

	const uint16_t length = h.length;
	if (length % attribute_alignment != 0)
	{
		return make_unexpected(errc::unexpected_message_length);
	}
	if (header_size + length > received)
	{
		return make_unexpected(errc::incomplete);
	}

	constexpr auto attribute_header_size = sizeof(__wire::type_and_length);
	auto attributes = buffer.subspan(header_size, length);
	while (!attributes.empty())
	{
		const auto &attribute = pal::packed_as<__wire::type_and_length>(
			attributes.first<attribute_header_size>()
		);
		const auto padded_length = __wire::align_up(attribute.length, attribute_alignment);
		if (padded_length > attributes.size() - attribute_header_size)
		{
			return make_unexpected(errc::unexpected_attribute_length);
		}
		attributes = attributes.subspan(attribute_header_size + padded_length);
	}

	return parsed_message<stun>{buffer};
}

} // namespace turner
