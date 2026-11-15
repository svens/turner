#include <turner/codec/msturn.hpp>
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

struct header_and_cookie
{
	pal::big_endian<uint16_t> type;
	pal::big_endian<uint16_t> length;
	msturn::transaction_id transaction_id;
	std::array<std::byte, 8> cookie;
};

static_assert(pal::packed_struct<header_and_cookie>);
static_assert(sizeof(header_and_cookie) == msturn::header_size + msturn::cookie.size());
static_assert(offsetof(header_and_cookie, transaction_id) == msturn::transaction_id_offset);
static_assert(offsetof(header_and_cookie, cookie) == msturn::cookie_offset);
static_assert(sizeof(header_and_cookie::cookie) == msturn::cookie.size());

} // namespace

pal::result<parsed_message<msturn>> msturn::parse (std::span<std::byte> buffer, size_t received) noexcept
{
	if (received < sizeof(header_and_cookie))
	{
		return make_unexpected(errc::incomplete);
	}

	const auto &h = pal::packed_as<header_and_cookie>(buffer.first<sizeof(header_and_cookie)>());
	if ((h.type & __wire::type_reserved_bits) != 0)
	{
		return make_unexpected(errc::unexpected_message_type);
	}
	if (h.cookie != cookie)
	{
		return make_unexpected(errc::invalid_magic_cookie);
	}

	const uint16_t length = h.length;
	if (length < cookie.size())
	{
		return make_unexpected(errc::unexpected_message_length);
	}
	if (header_size + length > received)
	{
		return make_unexpected(errc::incomplete);
	}

	constexpr auto attribute_header_size = sizeof(__wire::type_and_length);
	auto attributes = buffer.subspan(sizeof(header_and_cookie), length - cookie.size());
	while (!attributes.empty())
	{
		if (attributes.size() < attribute_header_size)
		{
			return make_unexpected(errc::unexpected_attribute_length);
		}
		const auto &attribute = pal::packed_as<__wire::type_and_length>(
			attributes.first<attribute_header_size>()
		);
		const size_t attribute_length = attribute.length;
		if (attribute_length > attributes.size() - attribute_header_size)
		{
			return make_unexpected(errc::unexpected_attribute_length);
		}
		attributes = attributes.subspan(attribute_header_size + attribute_length);
	}

	return parsed_message<msturn>{buffer};
}

} // namespace turner
